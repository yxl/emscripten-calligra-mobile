/* This file is part of the KDE project
  Copyright (C) 2007 Florian Piquemal <flotueur@yahoo.fr>
  Copyright (C) 2007 Alexis Ménard <darktears31@gmail.com>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#ifndef KPTPERTEDITOR_H
#define KPTPERTEDITOR_H

#include <kptviewbase.h>
#include <kptitemmodelbase.h>

#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QList>
#include <QHeaderView>
#include <QObject>
#include <QTreeWidget>
#include <QStringList>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QFont>

#include <QtCore/QVariant>
#include <QtGui>

#include <qwidget.h>

#include <kactionselector.h>

#include "kptpart.h"
#include "kpttask.h"
#include "kptcontext.h"
#include "kpttaskeditor.h"
#include <ui_kptperteditor.h>

#include "kptcommand.h"

#include "kptview.h"
#include "kptnode.h"

#include <kicon.h>
#include <kglobal.h>
#include <klocale.h>
#include <kprinter.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>

#include <kdebug.h>
class QTreeWidgetItem;


namespace KPlato
{

class View;
class Project;

class PertEditor : public ViewBase
{
    Q_OBJECT
public:

    PertEditor( Part *part, QWidget *parent );
    virtual void updateReadWrite( bool readwrite );
    void draw( Project &project);
    void drawSubTasksName( QTreeWidgetItem *parent,Node * currentNode);
    void loadRequiredTasksList(Node * taskNode);
    Node * itemToNode(QString itemName, Node * startNode);
    QList<Node*> listNodeNotView(Node * node);

signals:
    void refreshAvailableTaskList();

private slots:
    void dispAvailableTasks();
    void dispAvailableTasks( Relation *rel );
    void addTaskInRequiredList(QListWidgetItem * currentItem);
    void removeTaskFromRequiredList(QListWidgetItem * currentItem);
    void slotUpdate();


private:
    QTreeWidget * m_tasktree;
    KActionSelector * m_assignList;
    Part * m_part;
    Node * m_node;
    Project * m_project;
    QList<Node *> list_nodeNotView;
    Ui::PertEditor widget;
};

}  //KPlato namespace

#endif
