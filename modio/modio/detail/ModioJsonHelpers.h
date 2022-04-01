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
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/core/ModioLogger.h"
#include <nlohmann/json.hpp>

namespace Modio
{
	

	namespace Detail
	{
		class Buffer;
		class DynamicBuffer;

		template<typename T>
		inline bool ParseSafe(const nlohmann::json& Json, T& OutVar, const std::string& Key)
		{
			if (Json.contains(Key) && !Json.at(Key).is_null())
			{
				Json.at(Key).get_to<T>(OutVar);
				return true;
			}
			else
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Core,
											"Failed to deserialize json Key: {}", Key);
				return false;
			}
		}

		template<>
		inline bool ParseSafe<Modio::filesystem::path>(const nlohmann::json& Json, Modio::filesystem::path& OutVar,
													   const std::string& Key)
		{
			if (Json.contains(Key) && !Json.at(Key).is_null())
			{
				std::string PathString = Json.at(Key).get<std::string>();
				OutVar = Modio::filesystem::path(PathString);
				return true;
			}
			else
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Core,
											"Failed to deserialize json Key: {}", Key);
				return false;
			}
		}

		template<typename T>
		inline void from_json(const nlohmann::json& Json, std::shared_ptr<T>& Object)
		{
			if (Object)
			{
				from_json(Json, *Object);
			}
		}

		
		template<typename T>
		inline bool ParseSubobjectSafe(const nlohmann::json& Json, T& OutVar, const std::string& SubobjectKey,
									   const std::string& Key)
		{
			if (Json.contains(SubobjectKey))
			{
				const nlohmann::json& Subobject = Json.at(SubobjectKey);
				if (!Subobject.is_null())
				{
					return ParseSafe(Subobject, OutVar, Key);
				}
				else
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Core,
												"Subobject is null for SubobjectKey: {}", SubobjectKey);
				}
			}
			else
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Core,
											"Json does not contain SubobjectKey: {}", SubobjectKey);
			}
			return false;
		}

		

		MODIO_IMPL bool GetSubobjectSafe(const nlohmann::json& Json, const std::string& SubobjectKey,
							  nlohmann::json& OutSubobjectJson);

		MODIO_IMPL bool ParseArraySizeSafe(const nlohmann::json& Json, std::size_t& OutVar, const std::string& Key);


		MODIO_IMPL nlohmann::json ToJson(const Modio::Detail::Buffer& InBuffer);

		MODIO_IMPL nlohmann::json ToJson(const Modio::Detail::DynamicBuffer& ResponseBuffer);

		MODIO_IMPL nlohmann::json ToJson(const Modio::filesystem::path& Path);

		template<typename T>
		inline Modio::Optional<T> TryMarshalResponse(Modio::Detail::DynamicBuffer& ResponseBuffer)
		{
			// @todonow: I have managed to get in results of size 1 here that crash from ListAllMods
			T ResultStructure;

			auto ParsedJson = ToJson(ResponseBuffer);
			if (ParsedJson.is_discarded())
			{
				return {};
			}

			using nlohmann::from_json;
			from_json(ParsedJson, ResultStructure);
			return ResultStructure;
		}

		template<typename T>
		inline Modio::Optional<T> MarshalSubobjectResponse(std::string SubobjectName,
														   Modio::Detail::DynamicBuffer& ResponseBuffer)
		{
			T ResultStructure;
			using nlohmann::from_json;

			nlohmann::json Json = ToJson(ResponseBuffer);
			if (Json.is_discarded())
			{
				return {};
			}

			nlohmann::json SubobjectJson;
			if (GetSubobjectSafe(Json, SubobjectName, SubobjectJson))
			{
				from_json(SubobjectJson, ResultStructure);
			}
			else
			{
				return {};
			}

			return ResultStructure;
		}
	} // namespace Detail
} // namespace Modio

// Moved to here for ADL
namespace ghc
{
	namespace filesystem
	{
		inline void to_json(nlohmann::json& Json, const Modio::filesystem::path& Path)
		{
			using nlohmann::to_json;
			return to_json(Json, Path.u8string());
		}

		inline void from_json(const nlohmann::json& Json, Modio::filesystem::path& Path)
		{
			Path = Modio::filesystem::path(Json.get<std::string>());
		}
	} // namespace filesystem
} // namespace ghc

#ifndef MODIO_SEPARATE_COMPILATION
#include "modio/detail/ModioJsonHelpers.ipp"
#endif