/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

#pragma once
#include <asio/yield.hpp>

class DecompressorOp
{
	Modio::Detail::DynamicBuffer Input;
	Modio::Detail::DynamicBuffer Output;
	Modio::Detail::DynamicBitStream InputBitView;
	enum class BlockType
	{
		NotCompressed = 0,
		StaticHuffman = 1,
		DynamicHuffman = 2,
		Invalid = 3,
	};

	enum class DecompressorState
	{
		BlockEnded,
		InBlock
	};

	DecompressorState CurrentState = DecompressorState::BlockEnded;

	BlockType CurrentBlockType = BlockType::Invalid;
	bool bFinalBlockFound = false;
	struct
	{
		std::size_t BytesRemaining = 0;
	} NonCompressedState;

	struct
	{
	} StaticHuffmanState;

	struct
	{
	} DynamicHuffmanState;

	std::uint16_t DecodeLitlen() {

	}

	std::uint16_t DecodeDistance() {

	}
	void DetectBlockType()
	{
		std::uint8_t FinalBlock = InputBitView.ReadBits(1);
		std::uint8_t BlockTypeRaw = InputBitView.ReadBits(2);
		bFinalBlockFound = (FinalBlock != 0);
		CurrentBlockType = static_cast<BlockType>(BlockTypeRaw);
		if (CurrentBlockType != BlockType::Invalid)
		{
			CurrentState = DecompressorState::InBlock;
		}
	}
	void ProcessNonCompressedBytes()
	{
		std::size_t NumberOfBytesToRead = std::min(NonCompressedState.BytesRemaining, InputBitView.Size());

		Output.AppendBuffer(InputBitView.ReadWholeBytes(NumberOfBytesToRead));
		//need to put this also in the 'backwards window'
		NonCompressedState.BytesRemaining -= NumberOfBytesToRead;
		if (NonCompressedState.BytesRemaining == 0)
		{
			CurrentState = DecompressorState::BlockEnded;
		}
		else
		{
			CurrentState = DecompressorState::InBlock;
		}
	}

public:
	DecompressorOp(Modio::Detail::DynamicBuffer Input) : InputBitView(Input), Output(), bFinalBlockFound(false) {};

	Modio::Detail::DynamicBuffer GetInput()
	{
		return Input;
	}
	Modio::Detail::DynamicBuffer GetOutput()
	{
		return Output;
	}

	bool bCompleted()
	{
		return bFinalBlockFound;
	}

	Modio::ErrorCode Decompress()
	{
		if (Input.size() == 0)
		{
			// return not enough data error
		}

		switch (CurrentState)
		{
			case DecompressorState::BlockEnded:
			{
				DetectBlockType();

				if (CurrentBlockType == BlockType::Invalid)
				{
					// return invalid block error
				}

				switch (CurrentBlockType)
				{
					case BlockType::NotCompressed:
						Modio::Detail::Buffer Lengths = InputBitView.ReadWholeBytes(4);
						NonCompressedState.BytesRemaining = *((std::uint16_t*) Lengths.Data());
						ProcessNonCompressedBytes();

						break;
					case BlockType::StaticHuffman:
						break;
					case BlockType::DynamicHuffman:
						break;
					default:
						// return zip error
				}
			}
			break;
			case DecompressorState::InBlock:
			{
				switch (CurrentBlockType)
				{
					case BlockType::NotCompressed:
						ProcessNonCompressedBytes();
						break;
					case BlockType::StaticHuffman:
						break;
					case BlockType::DynamicHuffman:
						break;
				}
			}
			break;

		}

		// once we're done, we need to call consume on the input queue
		// this stops our memory from getting too big
	}
};

#include <asio/unyield.hpp>