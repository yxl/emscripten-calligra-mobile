/* This file is part of the KDE project
   Copyright (C) 2002 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KUDESIGNER_COMMAND_H
#define KUDESIGNER_COMMAND_H


/*    void slotAddDetailFooter();
    void slotAddDetail();
    void slotAddDetailHeader();
    void slotAddPageFooter();
    void slotAddPageHeader();
    void slotAddReportFooter();
    void slotAddReportHeader();
    void slotAddItemLine();
    void slotAddItemCalculated();
    void slotAddItemSpecial();
    void slotAddItemField();
    void slotAddItemLabel();*/
#include <kcommand.h>

class KudesignerDoc;
class CanvasDetailFooter;
class CanvasDetail;
class CanvasDetailHeader;
class CanvasPageFooter;
class CanvasPageHeader;
class CanvasReportFooter;
class CanvasReportHeader;
class CanvasBand;
class CanvasReportItem;
class ReportCanvas;

class AddDetailFooterCommand: public KNamedCommand{
public:
    AddDetailFooterCommand(int level, KudesignerDoc *doc);

    virtual void execute();
    virtual void unexecute();

private:
    int m_level;
    KudesignerDoc *m_doc;
    CanvasDetailFooter *m_section;
};

class AddDetailCommand: public KNamedCommand{
public:
    AddDetailCommand(int level, KudesignerDoc *doc);

    virtual void execute();
    virtual void unexecute();

private:
    int m_level;
    KudesignerDoc *m_doc;
    CanvasDetail *m_section;
};

class AddDetailHeaderCommand: public KNamedCommand{
public:
    AddDetailHeaderCommand(int level, KudesignerDoc *doc);

    virtual void execute();
    virtual void unexecute();

private:
    int m_level;
    KudesignerDoc *m_doc;
    CanvasDetailHeader *m_section;
};

class AddPageFooterCommand: public KNamedCommand{
public:
    AddPageFooterCommand(KudesignerDoc *doc);

    virtual void execute();
    virtual void unexecute();

private:
    KudesignerDoc *m_doc;
    CanvasPageFooter *m_section;
};

class AddPageHeaderCommand: public KNamedCommand{
public:
    AddPageHeaderCommand(KudesignerDoc *doc);

    virtual void execute();
    virtual void unexecute();

private:
    KudesignerDoc *m_doc;
    CanvasPageHeader *m_section;
};

class AddReportFooterCommand: public KNamedCommand{
public:
    AddReportFooterCommand(KudesignerDoc *doc);

    virtual void execute();
    virtual void unexecute();

private:
    KudesignerDoc *m_doc;
    CanvasReportFooter *m_section;
};

class AddReportHeaderCommand: public KNamedCommand{
public:
    AddReportHeaderCommand(KudesignerDoc *doc);

    virtual void execute();
    virtual void unexecute();

private:
    KudesignerDoc *m_doc;
    CanvasReportHeader *m_section;
};

class AddReportItemCommand: public KNamedCommand{
public:
    AddReportItemCommand(KudesignerDoc *doc, ReportCanvas *rc, int x, int y, CanvasBand *section);

    virtual void execute();
    virtual void unexecute();

private:
    int m_rtti;
    KudesignerDoc *m_doc;
    ReportCanvas *m_rc;
    int m_x;
    int m_y;
    CanvasReportItem *m_item;
    CanvasBand *m_section;
};

#endif
