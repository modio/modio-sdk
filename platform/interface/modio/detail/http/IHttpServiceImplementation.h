#pragma once

namespace Modio
{
	namespace Detail
	{
		class IHttpServiceImplementation
		{
		public:
			virtual ~IHttpServiceImplementation() {};

			virtual void Shutdown() = 0;

		};
	}
}