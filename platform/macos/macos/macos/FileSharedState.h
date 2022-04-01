/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once
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
			struct PendingIOOperation
			{
				enum class Direction
				{
					Read,
					Write
				};

				Modio::Detail::Buffer Data;
				int AssocFileDesc;
				bool DidFinish = false;
				Modio::Optional<Modio::ErrorCode> Result;
				Modio::FileSize NumBytesTransferred;
				Direction TransferDirection;
				Modio::FileOffset Offset;

				PendingIOOperation(Modio::Detail::Buffer Data, int FileDescriptor, Direction TransferDirection,
								   Modio::FileOffset Offset)
					: Data(std::move(Data)),
					  AssocFileDesc(FileDescriptor),
					  Result {},
					  NumBytesTransferred(0),
					  TransferDirection(TransferDirection),
					  Offset(Offset)
				{}
			};

			void PerformPendingOperation(PendingIOOperation& FileOp)
			{
				if (FileOp.DidFinish == true)
				{
					return;
				}

				// Stores the number of bytes transfered as result of pwrite/pread
				size_t Result = -1;
				// A flag that checks if the number of bytes exceeds MAX_BYTES, in case
				// it does, it will transfer MAX_BYTES at the time.
				bool Overflow = FileOp.Data.GetSize() > MAX_BYTES;
				// Shortcut to the FileSize
				size_t FileSize = FileOp.Data.GetSize();
				// Number of bytes to transfer in this operation
				size_t Bytes = Overflow ? MIN(MAX_BYTES, FileSize - FileOp.NumBytesTransferred) : FileSize;
				// The File offset, which could change according to the Overflow
				size_t AltFileOffset = FileOp.Offset;
				unsigned char* DataPointer;

				// When an operation requires to move more than MAX_BYTES, the
				// buffer is split in byte sections.
				if (Overflow)
				{
					// In case the next operation happens, it needs to move the pointer
					// by the number of bytes transferred las time
					DataPointer = FileOp.Data.Data() + FileOp.NumBytesTransferred;
					// Also the offset from the original location
					AltFileOffset += FileOp.NumBytesTransferred;
				}
				else
				{
					DataPointer = FileOp.Data.Data();
				}

				if (FileOp.TransferDirection == PendingIOOperation::Direction::Write)
				{
					Result = pwrite(FileOp.AssocFileDesc, DataPointer, Bytes, AltFileOffset);
				}
				else if (FileOp.TransferDirection == PendingIOOperation::Direction::Read)
				{
					Result = pread(FileOp.AssocFileDesc, DataPointer, Bytes, AltFileOffset);
				}

				if (Result < 0)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Could not operate on file descriptor {}, response: {}",
												FileOp.AssocFileDesc, Result);
					FileOp.Result = Modio::make_error_code(Modio::FilesystemError::FileLocked);
					FileOp.DidFinish = true;
				}

				// It is necessary to keep track of the number of bytes transferred in the Pending Operation
				FileOp.NumBytesTransferred += Modio::FileSize(Result);
				// The operation is considered finished if the number of bytes transferred are more or equal
				// to the expected Data size.
				FileOp.DidFinish = FileOp.NumBytesTransferred >= FileSize;

				return;
			}

			void HandlePendingTasks()
			{
				std::for_each(PendingIO.begin(), PendingIO.end(),
							  [this](std::pair<const int, PendingIOOperation>& Element) {
								  PendingIOOperation& PendingOp = Element.second;

								  // The PendingOperation has finished, nothing else to do.
								  if (PendingOp.DidFinish)
								  {
									  return;
								  }

								  size_t BytesTransfer = PendingOp.Data.GetSize() - PendingOp.NumBytesTransferred;

								  // It means that the PedingOp has not transferred all bytes to the Buffer
								  if (BytesTransfer > 0)
								  {
									  PerformPendingOperation(PendingOp);
								  }
								  else
								  {
									  PendingOp.Result = {};
									  PendingOp.DidFinish = true;
								  }
							  });
			};

			std::map<int, PendingIOOperation> PendingIO;
			const int MAX_BYTES = 1048575; // It operates in 1 MB chunks of data.

		public:
			Modio::ErrorCode Initialize()
			{
				return {};
			}

			~FileSharedState() {}

			/// @brief Adaptor function for converting Modio specific data structures to the underlying liburing C
			/// interface
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

				if (PendingOp.second == true)
				{
					PerformPendingOperation(PendingOp.first->second);
					return PendingOp.first->second.Result;
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
						IOStatus->second.NumBytesTransferred = Modio::FileSize(0);
						IOStatus->second.Data = std::move(SourceData);
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
				else
				{
					auto PendingOp = PendingIO.insert(std::make_pair(
						FileDescriptor, PendingIOOperation(std::move(SourceData), FileDescriptor,
														   PendingIOOperation::Direction::Write, OffsetInFile)));

					if (PendingOp.second == true)
					{
						PerformPendingOperation(PendingOp.first->second);
						return PendingOp.first->second.Result;
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
													"File Descriptor {} already in queue with path", FileDescriptor);
					}
				}

				return {};
			}

			std::pair<bool, Modio::Optional<Modio::ErrorCode>> IOCompleted(int FileDescriptor)
			{
				HandlePendingTasks();
				auto IOStatus = PendingIO.find(FileDescriptor);

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
				HandlePendingTasks();
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
												"Buffer read for File Descriptor {} with "
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
				HandlePendingTasks();
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
