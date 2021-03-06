/*
    Copyright (C) 2017 Aseman Team
    http://aseman.co

    TelegramStats is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TelegramStats is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASEMANCLIENTLIB_GLOBAL_H
#define ASEMANCLIENTLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
#if defined(ASEMANCLIENTLIB_LIBRARY)
#  define ASEMANCLIENTLIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ASEMANCLIENTLIBSHARED_EXPORT Q_DECL_IMPORT
#endif
#else
#define ASEMANCLIENTLIBSHARED_EXPORT
#endif

#endif // ASEMANCLIENTLIB_GLOBAL_H
