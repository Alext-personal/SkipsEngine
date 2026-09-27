    #pragma once
    #include "Resources/EditorTEMP/AssetRegistry.h"
    #include <filesystem>
    #include <efsw/efsw.hpp>
    namespace Gaze {
	    class GazeFileListener : public efsw::FileWatchListener {
	    public:
            GazeFileListener(AssetRegistry& registry):efsw::FileWatchListener(), m_registry(registry) {}
            void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                const std::string& filename, efsw::Action action,
                const std::string& oldFilename) override {
                AssetFileState state;
                switch (action) {
                case efsw::Actions::Add:
                    state = AssetFileState::Created;
                    break;
                case efsw::Actions::Delete:
                    state = AssetFileState::Deleted;
                    break;
                case efsw::Actions::Modified:
                    state = AssetFileState::Modified;
                    break;
                case efsw::Actions::Moved:
                    state = AssetFileState::Moved;
                    break;
                default:
                    std::cout << "Should never happen!" << std::endl;
                    return;
                }
                {
                    const std::lock_guard<std::mutex> lock(m_registry.importQueueMutex);
                    m_registry.importQueue.push(AssetFileData{ state,std::filesystem::path(dir) / filename,std::filesystem::path(dir) / oldFilename });
                }
               
            }
        private:
            AssetRegistry& m_registry;
	    };
        class AssetWatcher {
        public:
            AssetWatcher(AssetRegistry& registry) : m_registry(registry),m_fileListener(registry) {
            }
            void Watch() { 
                m_folderWatchID = m_fileWatcher.addWatch(m_registry.currentPath.string(), &m_fileListener, true);
                m_fileWatcher.watch();
            }
            void Pause() {
                m_fileWatcher.removeWatch(m_folderWatchID);
            }
            void Unpause() {
                m_folderWatchID = m_fileWatcher.addWatch(m_registry.currentPath.string(), &m_fileListener, true);
            }
            ~AssetWatcher() {
                m_fileWatcher.removeWatch(m_folderWatchID);
            }
            void ChangeDirectory(const std::filesystem::path& newPath) {
                m_fileWatcher.removeWatch(m_folderWatchID);
                m_folderWatchID = m_fileWatcher.addWatch(newPath.string(), &m_fileListener, true);
            }
        private:
            AssetRegistry& m_registry;
            efsw::FileWatcher m_fileWatcher;
            GazeFileListener m_fileListener;
            efsw::WatchID m_folderWatchID;
        };
    }