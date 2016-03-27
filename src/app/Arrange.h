/* Copyright (c) 2014-2015 "Omnidome" by cr8tr
 * Dome Mapping Projection Software (http://omnido.me).
 * Omnidome was created by Michael Winkelmann aka Wilston Oreo (@WilstonOreo)
 *
 * This file is part of Omnidome.
 *
 * Omnidome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <QWidget>
#include <omni/Session.h>
#include <omni/ui/mixin/DataModel.h>

namespace omni {
  namespace ui {
    class GLView3D;

    namespace Ui {
      class Arrange;
    }

    /// Arrange widget for positioning of elements and 3D view of scene
    class Arrange :
      public QWidget,
      public mixin::SharedDataModel<Session>{
        Q_OBJECT
        OMNI_UI_SHARED_DATAMODEL(Session)
      public:
        Arrange(QWidget * = nullptr);
        ~Arrange();

        /// Return pointer to view
        GLView3D      * view();

        /// Return pointer to view (const version)
        GLView3D const* view() const;

      signals:
        void            dataModelChanged();

      private:
        void            dataToFrontend();
        bool            frontendToData();

        std::unique_ptr<Ui::Arrange> ui_;
    };
  }
}
