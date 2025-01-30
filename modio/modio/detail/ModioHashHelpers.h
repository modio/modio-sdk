#pragma once

#include "hmac_sha256.h"
#include "modio/core/ModioStdTypes.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>


namespace Modio
{
	namespace Detail
	{
		namespace Hash
		{
			inline static std::string HMACSHA256String(const std::string& SecretKey, const std::string& StringToHash)
			{
				std::stringstream ss_result;
				std::vector<uint8_t> out(SHA256_HASH_SIZE);

				hmac_sha256(SecretKey.data(), SecretKey.size(), StringToHash.data(), StringToHash.size(), out.data(),
							out.size());

				for (uint8_t x : out)
				{
					ss_result << std::hex << std::setfill('0') << std::setw(2) << (int) x;
				}

				return ss_result.str();
			}
		}
	}
}