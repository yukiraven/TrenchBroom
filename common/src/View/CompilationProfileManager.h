/*
 Copyright (C) 2010-2016 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CompilationProfileManager_h
#define CompilationProfileManager_h

#include <wx/panel.h>

namespace TrenchBroom {
    namespace Model {
        class CompilationConfig;
    }
    
    namespace View {
        class CompilationProfileListBox;
        class CompilationProfileEditor;
        
        class CompilationProfileManager : public wxPanel {
        private:
            Model::CompilationConfig& m_config;
            CompilationProfileListBox* m_listView;
            CompilationProfileEditor* m_editor;
        public:
            CompilationProfileManager(wxWindow* parent, Model::CompilationConfig& config);
        private:
            void OnAddProfile(wxCommandEvent& event);
            void OnRemoveProfile(wxCommandEvent& event);
            void OnUpdateAddProfileButtonUI(wxUpdateUIEvent& event);
            void OnUpdateRemoveProfileButtonUI(wxUpdateUIEvent& event);
            
            void OnProfileSelectionChanged(wxCommandEvent& event);
        };
    }
}

#endif /* CompilationProfileManager_h */
