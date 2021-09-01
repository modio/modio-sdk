# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#   
# 

include(GetGitRevisionDescription)
find_package(Git)

function(GetVersionInfo _MainBranchName _MainCommitCount _BranchID _BranchCommitCount _CountDirtyChanges)
	
	#Abuse GetGitRevisionDescription commit detection to ensure commits are in sync
	get_git_head_revision(DummyRefSpec DummyHeadHash)

	#Calculate the commit count on the main branch
	execute_process(
				COMMAND "${GIT_EXECUTABLE}" rev-list --count "${_MainBranchName}" 
				WORKING_DIRECTORY "${MODIO_ROOT_DIR}"
				OUTPUT_VARIABLE MainCommitCount
				RESULT_VARIABLE MainCommitCountResult
				ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
			)


	set ("${_MainCommitCount}" "${MainCommitCount}" PARENT_SCOPE)
	#Empty default values if we are on the main branch
	set ("${_BranchID}" "" PARENT_SCOPE)
	set ("${_BranchCommitCount}" "" PARENT_SCOPE)	

	#detect if we are in a feature branch or not
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" branch --show-current 
		WORKING_DIRECTORY "${MODIO_ROOT_DIR}"
		OUTPUT_VARIABLE BranchName
		RESULT_VARIABLE BranchNameResult
		ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
	)


	if ("${BranchName}" STREQUAL "${_MainBranchName}")
		set (OnMainBranch TRUE)
		#Calculate the number of dirty files, update _BranchCommitCount if necessary
		
		execute_process(
			COMMAND "${GIT_EXECUTABLE}" status --porcelain
			WORKING_DIRECTORY "${MODIO_ROOT_DIR}"
			OUTPUT_VARIABLE DirtyFileList
			RESULT_VARIABLE DirtyState
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		string (LENGTH "${DirtyFileList}" DirtyListLength)

		if (DirtyListLength GREATER 0)
			string (REPLACE ";" "" DirtyFileList "${DirtyFileList}")
			string (REPLACE ":" "" DirtyFileList "${DirtyFileList}")

			# Turn lines into list by replacing newline with delimiter
			string (REPLACE "\n" ";" DirtyFileList "${DirtyFileList}")
	
			list (LENGTH DirtyFileList DirtyFileCount)
			if ("${_CountDirtyChanges}")
				set ("${_BranchCommitCount}" "d${DirtyFileCount}" PARENT_SCOPE)
			else()
				set ("${_BranchCommitCount}" "d" PARENT_SCOPE)
			endif()
					
		endif ()
	else()
	
		#Calculate the first two characters of the hash of the current branch name
		string (SHA1 BranchCommitHash "${BranchName}")
		string(SUBSTRING "${BranchCommitHash}" 0 2 BranchShortID)
		set ("${_BranchID}" "b${BranchShortID}" PARENT_SCOPE)

		#Calculate count of commits on feature branch
		execute_process(
			COMMAND "${GIT_EXECUTABLE}" merge-base "${BranchName}" "${_MainBranchName}"
			WORKING_DIRECTORY "${MODIO_ROOT_DIR}"
			OUTPUT_VARIABLE CommonAncestorHash
			RESULT_VARIABLE CommonAncestorHashResult
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		execute_process(
			COMMAND "${GIT_EXECUTABLE}" rev-list "${CommonAncestorHash}...HEAD" --count
			WORKING_DIRECTORY "${MODIO_ROOT_DIR}"
			OUTPUT_VARIABLE DivergedBranchCommitCount
			RESULT_VARIABLE DivergedBranchCommitCountResult
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		
		set ("${_BranchCommitCount}" "${DivergedBranchCommitCount}" PARENT_SCOPE)

		execute_process(
			COMMAND "${GIT_EXECUTABLE}" update-index -q --really-refresh
			WORKING_DIRECTORY "${MODIO_ROOT_DIR}"
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE ECHO_OUTPUT_VARIABLE
		)

		#Calculate the number of dirty files, update _BranchCommitCount if necessary
		execute_process(
			COMMAND "${GIT_EXECUTABLE}" status --porcelain
			WORKING_DIRECTORY "${MODIO_ROOT_DIR}"
			OUTPUT_VARIABLE DirtyFileList
			RESULT_VARIABLE DirtyState
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		string (LENGTH "${DirtyFileList}" DirtyListLength)

		if (DirtyListLength GREATER 0)
			string (REPLACE ";" "" DirtyFileList "${DirtyFileList}")
			string (REPLACE ":" "" DirtyFileList "${DirtyFileList}")

			# Turn lines into list by replacing newline with delimiter
			string (REPLACE "\n" ";" DirtyFileList "${DirtyFileList}")
	
			list (LENGTH DirtyFileList DirtyFileCount)
			if ("${_CountDirtyChanges}")
				set ("${_BranchCommitCount}" "${DivergedBranchCommitCount}d${DirtyFileCount}" PARENT_SCOPE)
			else()
				set ("${_BranchCommitCount}" "${DivergedBranchCommitCount}d" PARENT_SCOPE)
			endif()
					
		endif ()
	endif()
endfunction()
