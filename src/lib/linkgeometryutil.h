//
// Copyright 2014 wkhtmltopdf authors
//
// This file is part of wkhtmltopdf.
//
// wkhtmltopdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// wkhtmltopdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with wkhtmltopdf.  If not, see <http://www.gnu.org/licenses/>.

#ifndef LINKGEOMETRYUTIL_H
#define LINKGEOMETRYUTIL_H

class QWebFrame;
class LinkGeometryUtil
{
public:
    LinkGeometryUtil();

    static void outputErrorMessage(int statusCode, const char * errorMsg);
    static void outputGeometry(const QWebFrame * frame);
};

#endif // LINKGEOMETRYUTIL_H
