/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#pragma once

#include "StandardHeaders.h"
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/unordered_map.h>
#include "Command.h"
#include "CommandGroup.h"


namespace MCore
{
    // forward declarations
    class CommandManagerCallback;

    /**
     * The command manager.
     * This manager has a set of registered commands (MCore::Command) that can be executed and undo-ed and redo-ed through this manager.
     * Also it keeps a command history. It also supports the usage of command groups, which batch sets of commands as single commands inside the command history.
     */
    class MCORE_API CommandManager
    {
        MCORE_MEMORYOBJECTCATEGORY(CommandManager, MCORE_DEFAULT_ALIGNMENT, MCORE_MEMCATEGORY_COMMANDSYSTEM)

    public:
        /**
         * The command history entry stores all information which is relevant for a called command.
         * This information is needed for the undo/redo usage.
         */
        struct MCORE_API CommandHistoryEntry
        {
            CommandHistoryEntry()
                : mCommandGroup(nullptr)
                , mExecutedCommand(nullptr)
                , mParameters(nullptr) {}

            /**
             * Extended Constructor.
             * @param group The command group pointer. When set to nullptr it will use the single command object instead of the group.
             * @param command The command instance that has been created at execution time. When set to nullptr it will assume it is a group, and it will use the group you specified.
             * @param parameters The command arguments.
             */
            CommandHistoryEntry(CommandGroup* group, Command* command, const CommandLine& parameters, AZ::u32 historyItemNr);

            ~CommandHistoryEntry();

            static AZStd::string ToString(CommandGroup* group, Command* command, AZ::u32 historyItemNr);
            AZStd::string ToString() const;

            CommandGroup*   mCommandGroup;          /**< A pointer to the command group, or nullptr when no group is used (in that case it uses a single command). */
            Command*        mExecutedCommand;       /**< A pointer to the command object, or nullptr when no command is used (in that case it uses a group). */
            CommandLine     mParameters;            /**< The used command arguments, unused in case no command is used (in that case it uses a group). */
            AZ::u32         m_historyItemNr;        /**< The global history item number. This number will neither change depending on the size of the history queue nor with undo/redo. */
        };


        /**
         * The default constructor.
         */
        CommandManager();

        /**
         * The destructor.
         */
        virtual ~CommandManager();

        /**
         * Execute a command.
         * @param command The command string including all arguments.
         * @param outCommandResult The return/result value of the command.
         * @param addToHistory When set to true it is being added to the command history and can be undone.
         * @param outExecutedCommand This will contain the pointer to the command that was executed, or nullptr when no command was excuted. Please note that the pointer
         *                           that will be stored in this parameter will be invalid when the command failed to execute.
         * @param outExecutedParamters This will contain the command line parameter list of the executed command.
         * @param[in] callFromCommandGroup True in case the command is called from a command group. False in case the command to be called is not part of a command group and called only by itself.
         * @return True if the command succeeded, false if not.
         */
        bool ExecuteCommand(const char* command, String& outCommandResult, bool addToHistory = true, Command** outExecutedCommand = nullptr, CommandLine* outExecutedParamters = nullptr, bool callFromCommandGroup = false, bool clearErrors = true, bool handleErrors = true);
        bool ExecuteCommand(const AZStd::string& command, AZStd::string& outCommandResult, bool addToHistory = true, Command** outExecutedCommand = nullptr, CommandLine* outExecutedParamters = nullptr, bool callFromCommandGroup = false, bool clearErrors = true, bool handleErrors = true);
        bool ExecuteCommandInsideCommand(const char* command, String& outCommandResult);
        bool ExecuteCommandInsideCommand(const AZStd::string& command, String& outCommandResult);

        /**
         * Execute a command group.
         * The group will contain a set of commands inside it which are seen as one command.
         * This is useful when you want to for example execute 10 commands, but only show these 10 commands inside the command history as one grouped command.
         * Undoing the group will undo all commands that are part of the group..
         * @param commandGroup The command group to execute.
         * @param outCommandResult The result of the group execution. This generally contains the last successful or failed command status.
         * @param addToHistory Set to true when you want to add this group to the history.
         * @result Returns true when ALL commands inside the group executed successfully, otherwise false is returned.
         */
        bool ExecuteCommandGroup(CommandGroup& commandGroup, String& outCommandResult, bool addToHistory = true, bool clearErrors = true, bool handleErrors = true);
        bool ExecuteCommandGroup(CommandGroup& commandGroup, AZStd::string& outCommandResult, bool addToHistory = true, bool clearErrors = true, bool handleErrors = true);
        bool ExecuteCommandGroupInsideCommand(CommandGroup& commandGroup, String& outCommandResult);

        /**
         * Undo the last executed command in the command history.
         * @param outCommandResult The return/result value of the command.
         * @return True if the undo succeeded, false if not.
         */
        bool Undo(String& outCommandResult);

        /**
         * Redo the last command which has been undoed.
         * @param outCommandResult The return/result value of the command.
         * @return True if the redo succeeded, false if not.
         */
        bool Redo(String& outCommandResult);

        /**
         * Register a command to the command manager. Each command has to be registered using this function before
         * using it. You can just new a command object when calling this function. The command manager will keep track
         * of the used memory and will destroy it automatically. E.g. MCORE_COMMANDMANAGER.RegisterCommand( new CreateBoxCommand() );.
         * @param command The command to register.
         * @result True if command registration succeeded, false it not.
         */
        bool RegisterCommand(Command* command);

        /**
         * Debug function to log the current command history.
         */
        void LogCommandHistory();

        /**
         * Find the command in the hash table.
         * @param commandName The name of the command.
         * @return A pointer to the command object, nullptr if it hasn't been found.
         */
        Command* FindCommand(const AZStd::string& commandName);

        /**
         * Remove and delete all callbacks from memory.
         * Automatically called by the destructor.
         */
        void RemoveCallbacks();

        /**
         * Register a callback.
         * This increases the number returned by GetNumCallbacks().
         * @param callback The callback to add to the list of registered callbacks.
         */
        void RegisterCallback(CommandManagerCallback* callback);

        /**
         * Remove a given callback from the manager.
         * @param callback The callback to remove.
         * @param delFromMem Set to true when you wish to also delete the callback object from memory.
         */
        void RemoveCallback(CommandManagerCallback* callback, bool delFromMem = true);

        /**
         * Get the number of registered callbacks.
         * @result The number of registered callbacks.
         */
        uint32 GetNumCallbacks() const;

        /**
         * Get a given callback.
         * @param index The callback number to get, which must be in range of [0..GetNumCallbacks()-1].
         */
        CommandManagerCallback* GetCallback(uint32 index);

        /**
         * Set the maximum number of history items that the manager should remember.
         * On default this value is 100. This means it will remember the last 100 executed commands, which can then be undo-ed and redo-ed.
         * @param maxItems The maximum number of items to remember.
         */
        void SetMaxHistoryItems(uint32 maxItems);

        /**
         * Get the maximum number of history items that the manager will remember.
         * On default this value is 100. This means it will remember the last 100 executed commands, which can then be undo-ed and redo-ed.
         * @result The number of maximum history items.
         */
        uint32 GetMaxHistoryItems() const;

        /**
         * Get the current history index.
         * This value will be in range of [0..GetMaxHistoryItems()-1].
         * @result The current history index.
         */
        int32 GetHistoryIndex() const;

        /**
         * Get the number of history items stored.
         * This is the number of executed commands that are stored in the history right now.
         * @result The number of history items currently stored.
         */
        uint32 GetNumHistoryItems() const;

        const CommandHistoryEntry& GetHistoryItem(uint32 index) const;

        /**
         * Get a given command from the command history.
         * @param historyIndex The history index number, which must be in range of [0..GetNumHistoryItems()-1].
         * @result A pointer to the command stored at the given history index.
         */
        Command* GetHistoryCommand(uint32 historyIndex);

        /**
         * Clear the history.
         */
        void ClearHistory();

        /**
         * Get the command line for a given history item.
         * @param historyIndex The history index number, which must be in range of [0..GetNumHistoryItems()-1].
         * @result A reference to the command line that was used when executing this command.
         */
        const CommandLine& GetHistoryCommandLine(uint32 historyIndex) const;

        /**
         * Get the total number of registered commands.
         * @result The number of registered commands.
         */
        uint32 GetNumRegisteredCommands() const;

        /**
         * Get a given registered command.
         * @param index The command number, which must be in range of [0..GetNumRegisteredCommands()-1].
         * @result A pointer to the command.
         */
        Command* GetCommand(uint32 index);

        /**
         * Remove a given command callback. This automatically finds the command where this callback has been added to and removes it from that.
         * @param callback The callback to remove.
         * @param delFromMem When set to true, the specified callback will be deleted from memory as well (even when not registered at any of the commands).
         */
        void RemoveCommandCallback(Command::Callback* callback, bool delFromMem);

        /**
         * Remove a given command callback from a given command with a given name.
         * @param commandName The non-case-sentive name of the command to remove the callback from.
         * @param callback The callback to remove from the command with the specified name.
         * @param delFromMem When set to true, the specified callback will be deleted from memory as well (even when not registered at any of the commands).
         */
        void RemoveCommandCallback(const char* commandName, Command::Callback* callback, bool delFromMem);

        /**
         * Register (add) a command callback to a given command with a given name.
         * @param commandName The non-case-sensitive name of the command where we should add this callback to.
         * @param callback The callback to add to the given command.
         * @result Returns true when added successfully or false when there is no command with the specified name.
         */
        bool RegisterCommandCallback(const char* commandName, Command::Callback* callback);

        /**
         * Add error message to the internal callback based error handling system.
         * @param[in] errorLine The error line to add to the internal error handler.
         */
        MCORE_INLINE void AddError(const char* errorLine)                                       { mErrors.push_back(errorLine); }
        MCORE_INLINE void AddError(const AZStd::string& errorLine)                              { mErrors.push_back(errorLine); }

        /**
         * Checks if an error occurred and calls the error handling callbacks.
         * @result True in case an error occurred, false if not.
         */
        bool ShowErrorReport();

    protected:
        AZStd::unordered_map<AZStd::string, Command*>   mRegisteredCommands;    /**< A hash table storing the command objects for fast command object access. */
        Array<CommandHistoryEntry>                      mCommandHistory;        /**< The command history stack for undo/redo functionality. */
        Array<CommandManagerCallback*>                  mCallbacks;             /**< The command manager callbacks. */
        AZStd::vector<AZStd::string>                    mErrors;                /**< List of errors that happened during command execution. */
        Array<Command*>                                 mCommands;              /**< A flat array of registered commands, for easy traversal. */
        uint32                                          mMaxHistoryEntries;     /**< The maximum remembered commands in the command history. */
        int32                                           mHistoryIndex;          /**< The command history iterator. The current position in the undo/redo history. */
        AZ::u32                                         m_totalNumHistoryItems; /**< The number of history items since the application start. This number will neither change depending on the size of the history queue nor with undo/redo. */

        /**
         * Internal method to execute a command.
         * @param command The registered command object.
         * @param commandLine The commandline object.
         * @param outCommandResult The return/result value of the command.
         * @param addToHistory When set to true it is being added to the command history. This is set to false when redoing a command.
         * @return True if the command succeeded, false if not.
         */
        bool ExecuteCommand(Command* command, const CommandLine& commandLine, String& outCommandResult, bool addToHistory = true, bool clearErrors = true, bool handleErrors = true);

        /**
         * Push a command to the command history stack . This method will be automatically called by the system when
         * a command has been called but only if it is undoable.
         * @param command The command instance that has been created at execution time.
         * @param parameters The command arguments.
         */
        void PushCommandHistory(Command* command, const CommandLine& parameters);

        /**
         * Add the given command group to the history.
         * @param commandGroup The command group to add to the history.
         */
        void PushCommandHistory(CommandGroup* commandGroup);

        /**
         * Pop a command history item from the stack. After this the command cannot be un- or redone.
         */
        void PopCommandHistory();

        /**
         * Will be internally called before and after undoing a command.
         * @param command The command which will get or got executed.
         * @param parameters The given command parameters.
         * @param preUndo Execute all pre-undo callbacks in case this parameter is true, execute all post-undo callbacks elsewise.
         */
        void ExecuteUndoCallbacks(Command* command, const CommandLine& parameters, bool preUndo);

        /**
         * Will be internally called before and after executing a command.
         * @param command The command which will get or got executed.
         * @param parameters The given command parameters.
         * @param preCommand Execute all pre-commant callbacks in case this parameter is true, execute all post-command callbacks elsewise.
         */
        void ExecuteCommandCallbacks(Command* command, const CommandLine& parameters, bool preCommand);
    };
} // namespace MCore