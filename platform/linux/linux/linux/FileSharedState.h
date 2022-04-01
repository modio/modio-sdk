/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once

#include "linux/LibUringWrapper.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioErrorCode.h"
#include <map>

namespace Modio
{
	namespace Detail
	{
		class FileSharedState
		{
			// may need to be public but we'll see
			void UringHandlePendingCompletions()
			{
				io_uring_cqe* CompletionInfo;

				while (io_uring_peek_cqe(&UringState, &CompletionInfo) == 0)
				{
					int FileDescriptorVal = (int) (uintptr_t)(io_uring_cqe_get_data(CompletionInfo));
					// When the value of IOResult is negative, it means uring had an error. A positive
					// number refers to the number of bytes transacted
					int IOResult = CompletionInfo->res;
					auto IOStatus = PendingIO.find(FileDescriptorVal);

					if (IOStatus->second.DidFinish != true)
					{
						Modio::Optional<Modio::ErrorCode> Empty = {};
						if (IOStatus->second.TransferDirection == PendingIOOperation::Direction::Read)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
														"Read FileDescriptor: {}, Completion Info {}",
														FileDescriptorVal, IOResult);

							// Read may have only been partial, so make sure we record how much was read
							if (IOResult > 0)
							{
								std::size_t ExpectedBytes = IOStatus->second.Data.GetSize();
								std::size_t TotalBytes = IOResult + IOStatus->second.Offset;
								IOStatus->second.NumBytesTransferred = Modio::FileSize(IOResult);

								// The conditions to flag a "Finished" status are as follows:
								// - The peek operation has transfered to the Buffer all the ExpectedBytes
								// - The IOResult matches the number of ExpectedBytes.
								// The last case could occur when a Read/write ocurrs with offset and the expected
								// buffer is smaller than the TotalBytes.
								IOStatus->second.DidFinish = TotalBytes == ExpectedBytes || IOResult == ExpectedBytes;
								IOStatus->second.Result = Empty;

								// // NOTE!! R005a-Linux-Uring-Stall: When doing this task, some higher levels of the
								// SDK
								// // require that a read returns an "EndOfFile" error (ex.
								// ParseArchiveContentsOp.h@L52).
								// // At the moment, ReadSomeFromFileBufferedOp & ReadSomeFromFileOp has comments to
								// force
								// // the MaxBytesToRead. At the moment, if ExpectedBytes are more than IOResult or
								// TotalBytes
								// // the FileSharedState will bail reading more after 5 trials.
								// //
								// // This condition happens when the caller wants to read more bytes than the file has.
								// // The condition for a "64 * 1024" is related to file compression, which calls chunks
								// // of that size.
								// if (IOResult < ExpectedBytes && ExpectedBytes == (64 * 1024))
								// {
								// 	IOStatus->second.DidFinish = true;
								// 	IOStatus->second.Result = Modio::make_error_code(Modio::GenericError::EndOfFile);
								// }
							}
							else
							{
								IOStatus->second.DidFinish = true;
								IOStatus->second.Result = Modio::ErrorCode(IOResult * -1, std::system_category());
							}
						}
						else
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::File,
														"Write FileDescriptor: {}, Completion Info {}",
														FileDescriptorVal, IOResult);

							if (IOResult < 0)
							{
								IOStatus->second.DidFinish = true;
								IOStatus->second.Result = Modio::ErrorCode(IOResult * -1, std::system_category());
							}
							else if (IOResult == 0)
							{
								IOStatus->second.Result = Empty;
								IOStatus->second.DidFinish = true;
							}
							else
							{
								// we didn't write the entire buffer out, so we issue another call
								// Track bytes transferred and new offset
								IOStatus->second.NumBytesTransferred += Modio::FileSize(IOResult);
								IOStatus->second.Offset += Modio::FileOffset(IOResult);

								// Reissue the write request with the new range
								if (IOResult < IOStatus->second.Data.GetSize())
								{
									io_uring_sqe* NewOp = io_uring_get_sqe(&UringState);
									if (!NewOp)
									{
										return; // Will need to return an errorcode
									}
									// Prep another write call, offsetting into our existing buffer so we don't have to
									// reallocate
									io_uring_prep_write(
										NewOp, IOStatus->second.AssociatedDescriptor,
										IOStatus->second.Data.Data() + IOStatus->second.NumBytesTransferred,
										IOStatus->second.Data.GetSize() - IOStatus->second.NumBytesTransferred,
										IOStatus->second.Offset);

									io_uring_submit(&UringState);
								}

								IOStatus->second.Result = Empty;
								IOStatus->second.DidFinish = true;
							}
						}
					}

					io_uring_cqe_seen(&UringState, CompletionInfo);
					CompletionInfo = nullptr;
				}
			};

			struct PendingIOOperation
			{
				enum class Direction
				{
					Read,
					Write
				};

				Modio::Detail::Buffer Data;
				int AssociatedDescriptor;
				bool DidFinish = false;
				int OpTrials = 0;
				Modio::Optional<Modio::ErrorCode> Result;
				/// @brief In theory we should be configuring uring to never perform partial writes but we definitely
				/// need support for partial reads
				Modio::FileSize NumBytesTransferred;
				Direction TransferDirection;
				Modio::FileOffset Offset;
				PendingIOOperation(Modio::Detail::Buffer Data, int AssociatedDescriptor, Direction TransferDirection,
								   Modio::FileOffset Offset)
					: Data(std::move(Data)),
					  AssociatedDescriptor(AssociatedDescriptor),
					  Result {},
					  NumBytesTransferred(0),
					  TransferDirection(TransferDirection),
					  Offset(Offset)
				{}
			};

			std::map<int, PendingIOOperation> PendingIO;
			const int MAX_OP_TRIAL = 5;

		public:
			io_uring UringState;

			Modio::ErrorCode Initialize()
			{
				if (io_uring_queue_init(200, &UringState, 0))
				{
					return Modio::ErrorCode(errno, std::system_category());
				}
				else
				{
					return {};
				}
			}

			~FileSharedState()
			{
				// During testing, if the context is outside of normal execution, the UringState
				// is invalid, then it breaks when trying to "exit"
				if (UringState.sq.ring_ptr != NULL)
				{
					io_uring_queue_exit(&UringState);
				}
			}

			/// @brief Adaptor function for converting Modio specific data structures to the underlying liburing C
			/// interface
			/// @param SubmissionQueueEvent The SQE to associate with this IO request
			/// @param FileDescriptor The file handle to read from
			/// @param AmountOfData How much data to read
			/// @param OffsetInFile The absolute offset in the file to read from
			Modio::Optional<Modio::ErrorCode> SubmitRead(int FileDescriptor, Modio::FileSize AmountOfData,
														 Modio::FileOffset OffsetInFile)
			{
				if (AmountOfData <= 0)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Invalid submit read with File Descriptor {} and {} data",
												FileDescriptor, AmountOfData);
					return Modio::make_error_code(Modio::GenericError::BadParameter);
				}

				io_uring_sqe* NewOp = io_uring_get_sqe(&UringState);
				if (!NewOp)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Did not successfully submit read for File Descriptor {}",
												FileDescriptor);
					return Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle);
				}

				auto ExistingOp = PendingIO.find(FileDescriptor);
				if (ExistingOp != PendingIO.end())
				{
					// If PendingIO contains an operation with the flag finished,
					// it should be removed from the list because it is considered
					// a new call to read a file
					if (ExistingOp->second.DidFinish == true)
					{
						PendingIO.erase(ExistingOp);
					}
				}

				auto PendingOp = PendingIO.insert(std::make_pair(
					FileDescriptor, PendingIOOperation(Modio::Detail::Buffer(AmountOfData), FileDescriptor,
													   PendingIOOperation::Direction::Read, OffsetInFile)));
				if (PendingOp.second)
				{
					io_uring_prep_read(NewOp, FileDescriptor, PendingOp.first->second.Data.Data(), AmountOfData,
									   OffsetInFile);
					// Solve error: cast to 'void *' from smaller integer type 'int'
   	   				// With a cast to size_t
   					io_uring_sqe_set_data(NewOp, (void*)(size_t)FileDescriptor);
					if (!io_uring_submit(&UringState))
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
													"Did not successfully submit read for File Descriptor {}",
													FileDescriptor);
						return Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle);
					}
				}
				else
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
												"File Descriptor {} already in queue with path", FileDescriptor);
				}

				return {};
			}

			Modio::Optional<Modio::ErrorCode> SubmitWrite(int FileDescriptor, Modio::Detail::Buffer SourceData,
														  Modio::FileOffset OffsetInFile)
			{
				if (SourceData.GetSize() <= 0)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Invalid submit write with File Descriptor {} and {} data",
												FileDescriptor, SourceData.GetSize());
					return Modio::make_error_code(Modio::GenericError::BadParameter);
				}

				io_uring_sqe* NewOp = io_uring_get_sqe(&UringState);
				if (!NewOp)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Did not successfully submit write for File Descriptor {}",
												FileDescriptor);
					return Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle);
				}

				// When a file is downloaded, multiple write request occur all with the same File Descriptor. Because
				// a PendingIO is a dictionary, it keeps 1 FD for 1 PendingIOOperation.
				auto IOStatus = PendingIO.find(FileDescriptor);
				bool InsidePendingIO = IOStatus != PendingIO.end();
				if (InsidePendingIO == true)
				{
					// In case the previous PendingIOOperation finished, it is necessary to update the OffsetInFile to
					// the new value passed in this write operation.
					if (IOStatus->second.DidFinish == true)
					{
						IOStatus->second.DidFinish = false;
						IOStatus->second.Offset = OffsetInFile;
						IOStatus->second.OpTrials = 0;
					}
					// It is possible that the last operation did not work, then report back that error to the caller
					else if (IOStatus->second.Result.has_value())
					{
						return IOStatus->second.Result;
					}
					// If no error but still not finished, it will create an error which signals that the operation
					// is still in process.
					else
					{
						Modio::Detail::Logger().Log(
							Modio::LogLevel::Error, Modio::LogCategory::File,
							"Found a previous File Descriptor operation {} that has not finished", FileDescriptor);
						return {Modio::make_error_code(Modio::FilesystemError::FileLocked)};
					}
				}

				// For some reason, running this on Ubuntu 21.04 + liburing-dev 0.7-3ubuntu1, writing files with sizes
				// larger than 500KB would write suspicious bytes to the packages received from the internet. For that
				// reason, the line below Clones the original buffer with a dedicated alignment of 256.
				std::size_t Alignment = 256;
				Modio::Detail::Buffer AlignedData = SourceData.Clone(Alignment);
				// Calling this first so we don't have to worry about trying to access SourceData after we take
				// ownership by move
				io_uring_prep_write(NewOp, FileDescriptor, AlignedData.Data(), AlignedData.GetSize(), OffsetInFile);
				io_uring_sqe_set_data(NewOp, (void*)(size_t)FileDescriptor);

				if (InsidePendingIO == false)
				{
					PendingIO.insert(std::make_pair(
						FileDescriptor, PendingIOOperation(std::move(AlignedData), FileDescriptor,
														   PendingIOOperation::Direction::Write, OffsetInFile)));
				}
				else
				{
					// It is neccessary to move the AlignedData within the IOStatus to ensure the ownership of this
					// aligned data remains after its execution. Previous data can be discarded because it is
					// certain that the IO operation has finished.
					IOStatus->second.Data = std::move(AlignedData);
				}

				if (!io_uring_submit(&UringState))
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Did not successfully submit write for File Descriptor {}",
												FileDescriptor);
					return {Modio::make_error_code(Modio::GenericError::CouldNotCreateHandle)};
				}

				return {};
			}

			std::pair<bool, Modio::Optional<Modio::ErrorCode>> IOCompleted(int FileDescriptor)
			{
				UringHandlePendingCompletions();
				auto IOStatus = PendingIO.find(FileDescriptor);

				// This is a safeguard to avoid a situation when the read/writes uring function ends but
				// the IOStatus does not reach a "DidFinish == true" state
				if (IOStatus->second.OpTrials == MAX_OP_TRIAL)
				{
					Modio::Optional<Modio::ErrorCode> EC = {Modio::make_error_code(Modio::GenericError::EndOfFile)};
					return std::make_pair(true, EC);
				}
				else
				{
					IOStatus->second.OpTrials++;
				}

				if (IOStatus != PendingIO.end())
				{
					return std::make_pair(IOStatus->second.DidFinish, IOStatus->second.Result);
				}
				else
				{
					Modio::Optional<Modio::ErrorCode> empty = {};
					return std::make_pair(true, empty);
				}
			}

			Modio::Optional<Modio::Detail::Buffer> RetrieveReadBuffer(int FileDescriptor)
			{
				// Call this first to update any completions since we last checked
				UringHandlePendingCompletions();
				auto IOStatus = PendingIO.find(FileDescriptor);

				// The file descriptor is not present in the PendingIO dictionary
				if (IOStatus == PendingIO.end())
				{
					return {};
				}

				// Add a log case when there is an error. However, it is possible that the error
				// could be an "EndOfFile" condition.
				if (IOStatus->second.Result.has_value())
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Tried to retrieve read buffer for File Descriptor {} with "
												"NumBytesTransferred {} but the request had error {}",
												FileDescriptor, IOStatus->second.NumBytesTransferred,
												IOStatus->second.Result.value().value());
				}

				Modio::FileSize BytesTransferred = IOStatus->second.NumBytesTransferred;

				// In case the buffer had a read of more than 0 bytes, prepare to return
				if (BytesTransferred > 0)
				{
					Modio::Optional<Modio::Detail::Buffer> ReturnVal;

					// The function caller could allocate a buffer larger than the expected result, for
					// that reason, this condition creates a new buffer with only the final read data.
					// It is done here instead of "UringHandlePendingCompletions" because at that stage
					// it is possible to read more data. At this point we are sure we don't have any more
					// to read.
					if (BytesTransferred < IOStatus->second.Data.GetSize())
					{
						// Reallocate the buffer to the correct size so we don't have to report
						// NumBytesTransferred back to the caller and have them do the reallocation there
						Modio::Detail::Buffer ActualData = IOStatus->second.Data.CopyRange(0, BytesTransferred);
						ReturnVal = Modio::Optional<Modio::Detail::Buffer>(std::move(ActualData));
					}
					else
					{
						ReturnVal = Modio::Optional<Modio::Detail::Buffer>(std::move(IOStatus->second.Data));
					}

					if (IOStatus->second.DidFinish == true)
					{
						// The read operation finished, then it is not necesary to keep it
						// around in the PendingIO dictionary
						PendingIO.erase(IOStatus);
					}

					return ReturnVal;
				}
				else
				{
					return {};
				}
			}

			// returns error code if one happened, or an empty optional if still in progress?
			Modio::Optional<Modio::ErrorCode> RetrieveWriteResult(int FileDescriptor)
			{
				UringHandlePendingCompletions();
				auto IOStatus = PendingIO.find(FileDescriptor);
				if (IOStatus != PendingIO.end())
				{
					if (IOStatus->second.DidFinish == true)
					{
						// The write operation finished, then it is not necesary to keep it
						// around in the PendingIO dictionary
						PendingIO.erase(IOStatus);
					}

					if (IOStatus->second.Result.has_value())
					{
						return IOStatus->second.Result;
					}
				}

				// either we didn't find the descriptor (bad, maybe should return an error code instead) or we have a
				// write still in-flight (because Result is empty)
				return {};
			}
		};
	} // namespace Detail
} // namespace Modio
