/*
 *  Copyright (C) 2020-2022 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#include "modio/ModioSDK.h"

#include <iostream>
#include <utility>

/// This example extends 01_Initialization.cpp by demonstrating the functionality for
/// reporting games, users, or mods. This functionality is often a platform requirement. Reporting any of these
/// is possible via the ReportContentAsync function, which submits your report based on the parameters you provide, and
/// stores data validation errors if any occurred so you can retrieve them to display to users.

/// @brief Simple struct containing state variables for the example
struct ModioExampleFlags
{
	/// @brief Variable simulating a notification passed from a callback indicating the callback was invoked (ie the
	/// async SDK function returned a result)
	bool bDone = false;

	/// @brief Variable simulating a notification passed from a callback indicating whether the SDK function returned a
	/// success or failure
	bool bSuccess = false;

	/// @brief Keeps a reference to the last error received
	Modio::ErrorCode LastError;
};

/// @brief Static definition of the ModioExampleFlags. Consider a more sophisticated process
/// to deal with asynchronous calls should be designed in your game
static ModioExampleFlags Example;

/// @brief Static definition of the ModioExampleFlags. Consider a more sophisticated process
/// to deal with asynchronous calls should be designed in your game
struct ModioExample
{
	static bool HasAsyncOperationCompleted()
	{
		return std::exchange(Example.bDone, false);
	}

	static bool DidAsyncOperationSucceed()
	{
		return std::exchange(Example.bSuccess, false);
	}

	static void NotifyApplicationSuccess()
	{
		Example.bDone = true;
		Example.bSuccess = true;
	}

	static void NotifyApplicationFailure(Modio::ErrorCode ec)
	{
		Example.bDone = true;
		Example.bSuccess = false;
		std::cout << ec.message() << std::endl;
	}

	static void OnInitializeComplete(Modio::ErrorCode ec)
	{
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			NotifyApplicationSuccess();
		}
	}

	static void OnReportSubmitted(Modio::ErrorCode ec)
	{
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			NotifyApplicationSuccess();
		}
	}

	static void OnShutdownComplete(Modio::ErrorCode ec)
	{
		if (ec)
		{
			NotifyApplicationFailure(ec);
		}
		else
		{
			NotifyApplicationSuccess();
		}
	}
};

int main()
{
	Modio::InitializeAsync(Modio::InitializeOptions(Modio::GameID(3609),
													Modio::ApiKey("ca842a1f60c40bc8fb2044bc9932d763"),
													Modio::Environment::Live, Modio::Portal::None, "ExampleSession"),
						   ModioExample::OnInitializeComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		return -1;
	}

	// Populate the report with some example parameters. The first argument determines what type of report is being
	// submitted - game, mod, or user
	Modio::ReportParams Params =
		Modio::ReportParams(Modio::ModID(1), Modio::ReportType::DMCA, "This mod contains copyrighted content",
							"ReporterName", "Reporter@Email.com");
	Modio::ReportContentAsync(Params, ModioExample::OnReportSubmitted);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		std::cout << "Mod report operation did not succeed" << std::endl;
	}
	else
	{
		std::cout << "Mod report succeed" << std::endl;
	}

	Modio::ShutdownAsync(ModioExample::OnShutdownComplete);

	while (!ModioExample::HasAsyncOperationCompleted())
	{
		Modio::RunPendingHandlers();
	};

	if (!ModioExample::DidAsyncOperationSucceed())
	{
		// No need for explicit handling of an error from ShutdownAsync, the application is about to exit
	}
}
