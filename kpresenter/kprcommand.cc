/* This file is part of the KDE project
   Copyright (C) 2001 Laurent Montel <lmontel@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kpresenter_doc.h>
#include <kptextobject.h>
#include <kprcommand.h>
#include <kpbackground.h>
#include <kpclipartobject.h>
#include <kpgroupobject.h>


#include <kplineobject.h>
#include <kprectobject.h>
#include <kpellipseobject.h>
#include <kpautoformobject.h>

#include <kptextobject.h>
#include <kppixmapobject.h>

#include <kppartobject.h>
#include <koRuler.h>
#include <kppieobject.h>
#include <kprectobject.h>
#include <kpresenter_view.h>


/******************************************************************/
/* Class: TextCmd						  */
/******************************************************************/

/*======================== constructor ===========================*/
TextCmd::TextCmd(QString name, KPresenterDoc *doc, KPTextObject *tObj)
    : KCommand(name), document(doc), textObject(tObj) {
}

/*====================== execute =================================*/
void TextCmd::execute()
{
/*    textObject->ktextobject.redo();
      document->repaint(textObject);*/
}

/*====================== unexecute ===============================*/
void TextCmd::unexecute()
{
/*    textObject->ktextobject.undo();
      document->repaint(textObject);*/
}


/******************************************************************/
/* Class: ShadowCmd                                               */
/******************************************************************/

/*======================== constructor ===========================*/
ShadowCmd::ShadowCmd( QString _name, QPtrList<ShadowValues> &_oldShadow, ShadowValues _newShadow,
                      QPtrList<KPObject> &_objects, KPresenterDoc *_doc )
    : KCommand( _name ), oldShadow( _oldShadow ), objects( _objects )
{
    objects.setAutoDelete( false );
    oldShadow.setAutoDelete( false );
    doc = _doc;
    newShadow = _newShadow;

    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->incCmdRef();
}

/*======================== destructor ============================*/
ShadowCmd::~ShadowCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->decCmdRef();
    oldShadow.setAutoDelete( true );
    oldShadow.clear();
}

/*====================== execute =================================*/
void ShadowCmd::execute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
    {
        objects.at( i )->setShadowDistance( newShadow.shadowDistance );
        objects.at( i )->setShadowDirection( newShadow.shadowDirection );
        objects.at( i )->setShadowColor( newShadow.shadowColor );
    }
    doc->repaint( false );
}

/*====================== unexecute ===============================*/
void ShadowCmd::unexecute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
    {
        objects.at( i )->setShadowDistance( oldShadow.at( i )->shadowDistance );
        objects.at( i )->setShadowDirection( oldShadow.at( i )->shadowDirection );
        objects.at( i )->setShadowColor( oldShadow.at( i )->shadowColor );
    }
    doc->repaint( false );
}

/******************************************************************/
/* Class: SetOptionsCmd                                           */
/******************************************************************/

/*======================== constructor ===========================*/
SetOptionsCmd::SetOptionsCmd( QString _name, QPtrList<QPoint> &_diffs, QPtrList<KPObject> &_objects,
                              int _rastX, int _rastY, int _orastX, int _orastY,
                              QColor _txtBackCol, QColor _otxtBackCol, KPresenterDoc *_doc )
    : KCommand( _name ), diffs( _diffs ), objects( _objects ), txtBackCol( _txtBackCol ), otxtBackCol( _otxtBackCol )
{
    rastX = _rastX;
    rastY = _rastY;
    orastX = _orastX;
    orastY = _orastY;
    doc = _doc;
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->incCmdRef();
}

/*======================== destructor ============================*/
SetOptionsCmd::~SetOptionsCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->decCmdRef();
}

/*====================== execute =================================*/
void SetOptionsCmd::execute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->moveBy( *diffs.at( i ) );

    doc->setRasters( rastX, rastY, false );
    doc->setTxtBackCol( txtBackCol );
    doc->repaint( false );
}

/*====================== unexecute ===============================*/
void SetOptionsCmd::unexecute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->moveBy( -diffs.at( i )->x(), -diffs.at( i )->y() );

    doc->setRasters( orastX, orastY, false );
    doc->setTxtBackCol( otxtBackCol );
    doc->repaint( false );
}

/******************************************************************/
/* Class: SetBackCmd						  */
/******************************************************************/

/*======================== constructor ===========================*/
SetBackCmd::SetBackCmd( QString _name, QColor _backColor1, QColor _backColor2, BCType _bcType,
			bool _backUnbalanced, int _backXFactor, int _backYFactor,
			const KoImageKey & _backPix, const KPClipartKey & _backClip,
                        BackView _backView, BackType _backType,
			QColor _oldBackColor1, QColor _oldBackColor2, BCType _oldBcType,
			bool _oldBackUnbalanced, int _oldBackXFactor, int _oldBackYFactor,
			const KPClipartKey & _oldBackPix, const KPClipartKey & _oldBackClip,
                        BackView _oldBackView, BackType _oldBackType,
			bool _takeGlobal, int _currPgNum, KPresenterDoc *_doc )
    : KCommand( _name ), backColor1( _backColor1 ), backColor2( _backColor2 ), unbalanced( _backUnbalanced ),
      xfactor( _backXFactor ), yfactor( _backYFactor ), backPix( _backPix ), backClip( _backClip ),
      oldBackColor1( _oldBackColor1 ), oldBackColor2( _oldBackColor2 ), oldUnbalanced( _oldBackUnbalanced ),
      oldXFactor( _oldBackXFactor ), oldYFactor( _oldBackYFactor ), oldBackPix( _oldBackPix ), oldBackClip( _oldBackClip )
{
    bcType = _bcType;
    backView = _backView;
    backType = _backType;
    oldBcType = _oldBcType;
    oldBackView = _oldBackView;
    oldBackType = _oldBackType;
    takeGlobal = _takeGlobal;
    currPgNum = _currPgNum;
    doc = _doc;
}

/*====================== execute =================================*/
void SetBackCmd::execute()
{
    if ( !takeGlobal ) {
	doc->setBackColor( currPgNum - 1, backColor1, backColor2, bcType,
			   unbalanced, xfactor, yfactor );
	doc->setBackType( currPgNum - 1, backType );
	doc->setBackView( currPgNum - 1, backView );
	doc->setBackPixmap( currPgNum - 1, backPix );
	doc->setBackClipart( currPgNum - 1, backClip );
	doc->restoreBackground( currPgNum - 1 );
    } else {
	unsigned int i = 0;
	for ( i = 0; i < doc->getPageNums(); i++ ) {
	    doc->setBackColor( i, backColor1, backColor2, bcType,
			       unbalanced, xfactor, yfactor );
	    doc->setBackType( i, backType );
	    doc->setBackView( i, backView );
	    doc->setBackPixmap( i, backPix );
	    doc->setBackClipart( i, backClip );
	}

	for ( i = 0; i < doc->getPageNums(); i++ )
	    doc->restoreBackground( i );
    }

    doc->repaint( false );
}

/*====================== unexecute ===============================*/
void SetBackCmd::unexecute()
{
    if ( !takeGlobal ) {
	doc->setBackColor( currPgNum - 1, oldBackColor1, oldBackColor2, oldBcType,
			   oldUnbalanced, oldXFactor, oldYFactor );
	doc->setBackType( currPgNum - 1, oldBackType );
	doc->setBackView( currPgNum - 1, oldBackView );
	doc->setBackPixmap( currPgNum - 1, oldBackPix );
	doc->setBackClipart( currPgNum - 1, oldBackClip );
	doc->restoreBackground( currPgNum - 1 );
    } else {
	unsigned int i = 0;
	for ( i = 0; i < doc->getPageNums(); i++ ) {
	    doc->setBackColor( i, oldBackColor1, oldBackColor2, oldBcType,
			       oldUnbalanced, oldXFactor, oldYFactor );
	    doc->setBackType( i, oldBackType );
	    doc->setBackView( i, oldBackView );
	    doc->setBackPixmap( i, oldBackPix );
	    doc->setBackClipart( i, oldBackClip );
	}

	for ( i = 0; i < doc->getPageNums(); i++ )
	    doc->restoreBackground( i );
    }

    doc->repaint( false );
}

/******************************************************************/
/* Class: RotateCmd                                               */
/******************************************************************/

/*======================== constructor ===========================*/
RotateCmd::RotateCmd( QString _name, QPtrList<RotateValues> &_oldRotate, float _newAngle,
                      QPtrList<KPObject> &_objects, KPresenterDoc *_doc )
    : KCommand( _name ), oldRotate( _oldRotate ), objects( _objects )
{
    objects.setAutoDelete( false );
    oldRotate.setAutoDelete( false );
    doc = _doc;
    newAngle = _newAngle;

    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->incCmdRef();
}

/*======================== destructor ============================*/
RotateCmd::~RotateCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->decCmdRef();
    oldRotate.setAutoDelete( true );
    oldRotate.clear();
}

/*====================== execute =================================*/
void RotateCmd::execute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->rotate( newAngle );

    doc->repaint( false );
}

/*====================== unexecute ===============================*/
void RotateCmd::unexecute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->rotate( oldRotate.at( i )->angle );

    doc->repaint( false );
}

/******************************************************************/
/* Class: ChgClipCmd                                              */
/******************************************************************/

/*======================== constructor ===========================*/
ChgClipCmd::ChgClipCmd( QString _name, KPClipartObject *_object, KPClipartCollection::Key _oldKey,
                        KPClipartCollection::Key _newKey, KPresenterDoc *_doc )
    : KCommand( _name ), oldKey( _oldKey ), newKey( _newKey )
{
    object = _object;
    doc = _doc;
    object->incCmdRef();
}

/*======================== destructor ============================*/
ChgClipCmd::~ChgClipCmd()
{
    object->decCmdRef();
}

/*======================== execute ===============================*/
void ChgClipCmd::execute()
{
    object->setClipart( newKey );
    doc->repaint( object );
}

/*====================== unexecute ===============================*/
void ChgClipCmd::unexecute()
{
    object->setClipart( oldKey );
    doc->repaint( object );
}

/******************************************************************/
/* Class: ChgPixCmd                                               */
/******************************************************************/

/*======================== constructor ===========================*/
ChgPixCmd::ChgPixCmd( QString _name, KPPixmapObject *_oldObject, KPPixmapObject *_newObject,
                      KPresenterDoc *_doc )
    : KCommand( _name )
{
    oldObject = _oldObject;
    newObject = _newObject;
    doc = _doc;
    oldObject->incCmdRef();
    newObject->incCmdRef();
    newObject->setSize( oldObject->getSize() );
    newObject->setOrig( oldObject->getOrig() );
}

/*======================== destructor ============================*/
ChgPixCmd::~ChgPixCmd()
{
    oldObject->decCmdRef();
    newObject->decCmdRef();
}

/*======================== execute ===============================*/
void ChgPixCmd::execute()
{
    unsigned int pos = doc->objectList()->findRef( oldObject );
    doc->objectList()->take( pos );
    doc->objectList()->insert( pos, newObject );
    doc->repaint( newObject );
}

/*====================== unexecute ===============================*/
void ChgPixCmd::unexecute()
{
    unsigned int pos = doc->objectList()->findRef( newObject );
    doc->objectList()->take( pos );
    doc->objectList()->insert( pos, oldObject );
    doc->repaint( oldObject );
}

/******************************************************************/
/* Class: DeleteCmd						  */
/******************************************************************/

/*======================== constructor ===========================*/
DeleteCmd::DeleteCmd( QString _name, QPtrList<KPObject> &_objects, KPresenterDoc *_doc )
    : KCommand( _name ), objects( _objects )
{
    objects.setAutoDelete( false );
    doc = _doc;
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->incCmdRef();
}

/*======================== destructor ============================*/
DeleteCmd::~DeleteCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->decCmdRef();
}

/*======================== execute ===============================*/
void DeleteCmd::execute()
{
    QRect oldRect;

    for ( unsigned int i = 0; i < objects.count(); i++ )
    {
	oldRect = objects.at( i )->getBoundingRect( 0, 0 );
	if ( doc->objectList()->findRef( objects.at( i ) ) != -1 )
	{
	    doc->objectList()->take( doc->objectList()->findRef( objects.at( i ) ) );
	    objects.at( i )->removeFromObjList();
	}
	doc->repaint( oldRect );
	doc->repaint( objects.at( i ) );
    }
}

/*====================== unexecute ===============================*/
void DeleteCmd::unexecute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
    {
	doc->objectList()->append( objects.at( i ) );
	objects.at( i )->addToObjList();
	doc->repaint( objects.at( i ) );
    }
}


/******************************************************************/
/* Class: EffectCmd                                               */
/******************************************************************/

/*================================================================*/
EffectCmd::EffectCmd( QString _name, const QPtrList<KPObject> &_objs,
		      const QValueList<EffectStruct> &_oldEffects, EffectStruct _newEffect )
    : KCommand( _name ), oldEffects( _oldEffects ),
      newEffect( _newEffect ), objs( _objs )
{
    for ( unsigned int i = 0; i < objs.count(); ++i )
        objs.at( i )->incCmdRef();
}

/*================================================================*/
EffectCmd::~EffectCmd()
{
    for ( unsigned int i = 0; i < objs.count(); ++i )
        objs.at( i )->decCmdRef();
}

/*================================================================*/
void EffectCmd::execute()
{
    KPObject *object = 0;
    for ( unsigned int i = 0; i < objs.count(); ++i ) {
	object = objs.at( i );

	object->setPresNum( newEffect.presNum );
	object->setEffect( newEffect.effect );
	object->setEffect2( newEffect.effect2 );
	object->setDisappear( newEffect.disappear );
	object->setEffect3( newEffect.effect3 );
	object->setDisappearNum( newEffect.disappearNum );
    }
}

/*================================================================*/
void EffectCmd::unexecute()
{
    KPObject *object = 0;
    for ( unsigned int i = 0; i < objs.count(); ++i ) {
	object = objs.at( i );

	object->setPresNum( oldEffects[ i ].presNum );
	object->setEffect( oldEffects[ i ].effect );
	object->setEffect2( oldEffects[ i ].effect2 );
	object->setDisappear( oldEffects[ i ].disappear );
	object->setEffect3( oldEffects[ i ].effect3 );
	object->setDisappearNum( oldEffects[ i ].disappearNum );
    }
}

/******************************************************************/
/* Class: GroupObjCmd						  */
/******************************************************************/

/*==============================================================*/
GroupObjCmd::GroupObjCmd( const QString &_name,
			  const QPtrList<KPObject> &_objects,
			  KPresenterDoc *_doc )
    : KCommand( _name ), objects( _objects )
{
    objects.setAutoDelete( false );
    doc = _doc;
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->incCmdRef();
    grpObj = new KPGroupObject( objects );
    grpObj->incCmdRef();
}

/*==============================================================*/
GroupObjCmd::~GroupObjCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->decCmdRef();
    grpObj->decCmdRef();
}

/*==============================================================*/
void GroupObjCmd::execute()
{
    QRect r = objects.first()->getBoundingRect( 0, 0 );
    KPObject *obj = 0;

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	obj = objects.at( i );
	obj->setSelected( false );
	doc->objectList()->take( doc->objectList()->findRef( obj ) );
	obj->removeFromObjList();
	r = r.unite( obj->getBoundingRect( 0, 0 ) );
    }

    grpObj->setUpdateObjects( false );
    grpObj->setOrig( r.x(), r.y() );
    grpObj->setSize( r.width(), r.height() );
    doc->objectList()->append( grpObj );
    grpObj->addToObjList();
    grpObj->setUpdateObjects( true );
    grpObj->setSelected( true );

    doc->repaint( false );
}

/*==============================================================*/
void GroupObjCmd::unexecute()
{
    grpObj->setUpdateObjects( false );
    KPObject *obj = 0;

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	obj = objects.at( i );
	doc->objectList()->append( obj );
	obj->addToObjList();
	obj->setSelected( true );
    }

    doc->objectList()->take( doc->objectList()->findRef( grpObj ) );
    grpObj->removeFromObjList();

    doc->repaint( false );
}

/******************************************************************/
/* Class: UnGroupObjCmd						  */
/******************************************************************/

/*==============================================================*/
UnGroupObjCmd::UnGroupObjCmd( const QString &_name,
			  KPGroupObject *grpObj_,
			  KPresenterDoc *_doc )
    : KCommand( _name ), objects( grpObj_->getObjects() )
{
    objects.setAutoDelete( false );
    doc = _doc;
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->incCmdRef();
    grpObj = grpObj_;
    grpObj->incCmdRef();
}

/*==============================================================*/
UnGroupObjCmd::~UnGroupObjCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->decCmdRef();
    grpObj->decCmdRef();
}

/*==============================================================*/
void UnGroupObjCmd::execute()
{
    KPObject *obj = 0;
    grpObj->setUpdateObjects( false );

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	obj = objects.at( i );
	doc->objectList()->append( obj );
	obj->addToObjList();
	obj->setSelected( true );
    }

    doc->objectList()->take( doc->objectList()->findRef( grpObj ) );
    grpObj->removeFromObjList();

    doc->repaint( false );
}

/*==============================================================*/
void UnGroupObjCmd::unexecute()
{
    QRect r = objects.first()->getBoundingRect( 0, 0 );

    KPObject *obj = 0;

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	obj = objects.at( i );
	obj->setSelected( false );
	doc->objectList()->take( doc->objectList()->findRef( obj ) );
	obj->removeFromObjList();
	r = r.unite( obj->getBoundingRect( 0, 0 ) );
    }

    grpObj->setUpdateObjects( false );
    grpObj->setOrig( r.x(), r.y() );
    grpObj->setSize( r.width(), r.height() );
    doc->objectList()->append( grpObj );
    grpObj->addToObjList();
    grpObj->setUpdateObjects( true );
    grpObj->setSelected( true );

    doc->repaint( false );
}

/******************************************************************/
/* Class: InsertCmd						  */
/******************************************************************/

/*======================== constructor ===========================*/
InsertCmd::InsertCmd( QString _name, KPObject *_object, KPresenterDoc *_doc )
    : KCommand( _name )
{
    object = _object;
    doc = _doc;
    object->incCmdRef();
}

/*======================== destructor ============================*/
InsertCmd::~InsertCmd()
{
    object->decCmdRef();
}

/*====================== execute =================================*/
void InsertCmd::execute()
{
    doc->objectList()->append( object );
    object->addToObjList();
    if ( object->getType() == OT_TEXT )
	( (KPTextObject*)object )->recalcPageNum( doc );
    doc->repaint( object );
}

/*====================== unexecute ===============================*/
void InsertCmd::unexecute()
{
    QRect oldRect = object->getBoundingRect( 0, 0 );
    if ( doc->objectList()->findRef( object ) != -1 ) {
	doc->objectList()->take( doc->objectList()->findRef( object ) );
	object->removeFromObjList();
    }
    doc->repaint( oldRect );
}

/******************************************************************/
/* Class: LowerRaiseCmd                                           */
/******************************************************************/

/*======================== constructor ===========================*/
LowerRaiseCmd::LowerRaiseCmd( QString _name, QPtrList<KPObject> *_oldList, QPtrList<KPObject> *_newList, KPresenterDoc *_doc )
    : KCommand( _name )
{
    oldList = _oldList;
    newList = _newList;
    m_executed = false;
    oldList->setAutoDelete( false );
    newList->setAutoDelete( false );
    doc = _doc;

    for ( unsigned int i = 0; i < oldList->count(); i++ )
        oldList->at( i )->incCmdRef();
}

/*======================== destructor ============================*/
LowerRaiseCmd::~LowerRaiseCmd()
{
    for ( unsigned int i = 0; i < oldList->count(); i++ )
        oldList->at( i )->decCmdRef();

    // I'm not sure how to handle this here correctly ( to avoid memory leaks and not to crash... )
    //delete oldList;
    //delete newList;

    // David: well, it's simple: if execute() was done last, only oldList should be deleted
    // and if unexecute() was done last, only newList should be deleted
    // (reason: the doc - or another command - holds the other list).
    if ( m_executed )
        delete oldList;
    else
        delete newList;
}

/*====================== execute =================================*/
void LowerRaiseCmd::execute()
{
    doc->setObjectList( newList );
    doc->repaint( false );
    m_executed = true;
}

/*====================== unexecute ===============================*/
void LowerRaiseCmd::unexecute()
{
    doc->setObjectList( oldList );
    doc->repaint( false );
    m_executed = false;
}

/******************************************************************/
/* Class: MoveByCmd						  */
/******************************************************************/

/*======================== constructor ===========================*/
MoveByCmd::MoveByCmd( QString _name, QPoint _diff, QPtrList<KPObject> &_objects, KPresenterDoc *_doc )
    : KCommand( _name ), diff( _diff ), objects( _objects )
{
    objects.setAutoDelete( false );
    doc = _doc;
    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	if ( objects.at( i )->getType() == OT_TEXT ) {
	    ( (KPTextObject*)objects.at( i ) )->recalcPageNum( doc );
	    doc->repaint( objects.at( i ) );
	}
	objects.at( i )->incCmdRef();
    }
}

/*======================== destructor ============================*/
MoveByCmd::~MoveByCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->decCmdRef();
}

/*====================== execute =================================*/
void MoveByCmd::execute()
{
    QRect oldRect;

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	oldRect = objects.at( i )->getBoundingRect( 0, 0 );
	objects.at( i )->moveBy( diff );
	if ( objects.at( i )->getType() == OT_TEXT )
	    ( (KPTextObject*)objects.at( i ) )->recalcPageNum( doc );
	doc->repaint( oldRect );
	doc->repaint( objects.at( i ) );
    }
}

/*====================== unexecute ===============================*/
void MoveByCmd::unexecute()
{
    QRect oldRect;

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	oldRect = objects.at( i )->getBoundingRect( 0, 0 );
	objects.at( i )->moveBy( -diff.x(), -diff.y() );
	if ( objects.at( i )->getType() == OT_TEXT )
	    ( (KPTextObject*)objects.at( i ) )->recalcPageNum( doc );
	doc->repaint( oldRect );
	doc->repaint( objects.at( i ) );
    }
}

/******************************************************************/
/* Class: MoveByCmd2						  */
/******************************************************************/

/*======================== constructor ===========================*/
MoveByCmd2::MoveByCmd2( QString _name, QPtrList<QPoint> &_diffs,
			QPtrList<KPObject> &_objects, KPresenterDoc *_doc )
    : KCommand( _name ), diffs( _diffs ), objects( _objects )
{
    objects.setAutoDelete( false );
    diffs.setAutoDelete( true );
    doc = _doc;
    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	if ( objects.at( i )->getType() == OT_TEXT ) {
	    ( (KPTextObject*)objects.at( i ) )->recalcPageNum( doc );
	    doc->repaint( objects.at( i ) );
	}
	objects.at( i )->incCmdRef();
    }
}

/*======================== destructor ============================*/
MoveByCmd2::~MoveByCmd2()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->decCmdRef();

    diffs.clear();
}

/*====================== execute =================================*/
void MoveByCmd2::execute()
{
    QRect oldRect;

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	oldRect = objects.at( i )->getBoundingRect( 0, 0 );
	objects.at( i )->moveBy( *diffs.at( i ) );
	if ( objects.at( i )->getType() == OT_TEXT )
	    ( (KPTextObject*)objects.at( i ) )->recalcPageNum( doc );
	doc->repaint( oldRect );
	doc->repaint( objects.at( i ) );
    }
}

/*====================== unexecute ===============================*/
void MoveByCmd2::unexecute()
{
    QRect oldRect;

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	oldRect = objects.at( i )->getBoundingRect( 0, 0 );
	objects.at( i )->moveBy( -diffs.at( i )->x(), -diffs.at( i )->y() );
	if ( objects.at( i )->getType() == OT_TEXT )
	    ( (KPTextObject*)objects.at( i ) )->recalcPageNum( doc );
	doc->repaint( oldRect );
	doc->repaint( objects.at( i ) );
    }
}

/******************************************************************/
/* Class: PenBrushCmd						  */
/******************************************************************/

/*======================== constructor ===========================*/
PenBrushCmd::PenBrushCmd( QString _name, QPtrList<Pen> &_oldPen, QPtrList<Brush> &_oldBrush,
			  Pen _newPen, Brush _newBrush, QPtrList<KPObject> &_objects, KPresenterDoc *_doc, int _flags )
    : KCommand( _name ), oldPen( _oldPen ), oldBrush( _oldBrush ), objects( _objects )
{
    objects.setAutoDelete( false );
    oldPen.setAutoDelete( false );
    oldBrush.setAutoDelete( false );
    doc = _doc;
    newPen = _newPen;
    newBrush = _newBrush;
    flags = _flags;

    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->incCmdRef();
}

/*======================== destructor ============================*/
PenBrushCmd::~PenBrushCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
	objects.at( i )->decCmdRef();
    oldPen.setAutoDelete( true );
    oldPen.clear();
    oldBrush.setAutoDelete( true );
    oldBrush.clear();
}

/*====================== execute =================================*/
void PenBrushCmd::execute()
{
    KPObject *kpobject = 0L;
    Pen tmpPen = newPen;
    Brush tmpBrush = newBrush;

    for ( int i = 0; i < static_cast<int>( objects.count() ); i++ ) {
	if ( flags & LB_ONLY ) {
	    newPen.pen = oldPen.at( i )->pen;
	    newPen.lineEnd = oldPen.at( i )->lineEnd;
	    newBrush = *oldBrush.at( i );
	}
	if ( flags & LE_ONLY ) {
	    newPen.pen = oldPen.at( i )->pen;
	    newPen.lineBegin = oldPen.at( i )->lineBegin;
	    newBrush = *oldBrush.at( i );
	}
	if ( flags & PEN_ONLY ) {
	    newPen.lineBegin = oldPen.at( i )->lineBegin;
	    newPen.lineEnd = oldPen.at( i )->lineEnd;
	    if ( newPen.pen != Qt::NoPen )
		newPen.pen = QPen( newPen.pen.color(), oldPen.at( i )->pen.width(),
				   oldPen.at( i )->pen.style() != Qt::NoPen ? oldPen.at( i )->pen.style() : Qt::SolidLine );
	    else
		newPen.pen = QPen( oldPen.at( i )->pen.color(), oldPen.at( i )->pen.width(), Qt::NoPen );
	    newBrush = *oldBrush.at( i );
	}
	if ( flags & BRUSH_ONLY ) {
	    newBrush.gColor1 = oldBrush.at( i )->gColor1;
	    newBrush.gColor2 = oldBrush.at( i )->gColor2;
	    newBrush.unbalanced = oldBrush.at( i )->unbalanced;
	    newBrush.xfactor = oldBrush.at( i )->xfactor;
	    newBrush.yfactor = oldBrush.at( i )->yfactor;
	    if ( newBrush.brush != Qt::NoBrush )
		newBrush.brush = QBrush( newBrush.brush.color(),
					 oldBrush.at( i )->brush.style() != Qt::NoBrush ? oldBrush.at( i )->brush.style() : Qt::SolidPattern );
	    else
		newBrush.brush = QBrush( oldBrush.at( i )->brush.color(), Qt::NoBrush );
	    newBrush.gType = oldBrush.at( i )->gType;
	    newPen = *oldPen.at( i );
	}

	kpobject = objects.at( i );
	switch ( kpobject->getType() ) {
	case OT_LINE:
	    dynamic_cast<KPLineObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPLineObject*>( kpobject )->setLineBegin( newPen.lineBegin );
	    dynamic_cast<KPLineObject*>( kpobject )->setLineEnd( newPen.lineEnd );
	    doc->repaint( kpobject );
	    break;
	case OT_RECT:
	    dynamic_cast<KPRectObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPRectObject*>( kpobject )->setBrush( newBrush.brush );
	    dynamic_cast<KPRectObject*>( kpobject )->setFillType( newBrush.fillType );
	    dynamic_cast<KPRectObject*>( kpobject )->setGColor1( newBrush.gColor1 );
	    dynamic_cast<KPRectObject*>( kpobject )->setGColor2( newBrush.gColor2 );
	    dynamic_cast<KPRectObject*>( kpobject )->setGType( newBrush.gType );
	    dynamic_cast<KPRectObject*>( kpobject )->setGUnbalanced( newBrush.unbalanced );
	    dynamic_cast<KPRectObject*>( kpobject )->setGXFactor( newBrush.xfactor );
	    dynamic_cast<KPRectObject*>( kpobject )->setGYFactor( newBrush.yfactor );
	    doc->repaint( kpobject );
	    break;
	case OT_ELLIPSE:
	    dynamic_cast<KPEllipseObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPEllipseObject*>( kpobject )->setBrush( newBrush.brush );
	    dynamic_cast<KPEllipseObject*>( kpobject )->setFillType( newBrush.fillType );
	    dynamic_cast<KPEllipseObject*>( kpobject )->setGColor1( newBrush.gColor1 );
	    dynamic_cast<KPEllipseObject*>( kpobject )->setGColor2( newBrush.gColor2 );
	    dynamic_cast<KPEllipseObject*>( kpobject )->setGType( newBrush.gType );
	    dynamic_cast<KPEllipseObject*>( kpobject )->setGUnbalanced( newBrush.unbalanced );
	    dynamic_cast<KPEllipseObject*>( kpobject )->setGXFactor( newBrush.xfactor );
	    dynamic_cast<KPEllipseObject*>( kpobject )->setGYFactor( newBrush.yfactor );
	    doc->repaint( kpobject );
	    break;
	case OT_AUTOFORM:
	    dynamic_cast<KPAutoformObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setBrush( newBrush.brush );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setLineBegin( newPen.lineBegin );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setLineEnd( newPen.lineEnd );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setFillType( newBrush.fillType );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setGColor1( newBrush.gColor1 );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setGColor2( newBrush.gColor2 );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setGType( newBrush.gType );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setGUnbalanced( newBrush.unbalanced );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setGXFactor( newBrush.xfactor );
	    dynamic_cast<KPAutoformObject*>( kpobject )->setGYFactor( newBrush.yfactor );
	    doc->repaint( kpobject );
	    break;
	case OT_PIE:
	    dynamic_cast<KPPieObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPPieObject*>( kpobject )->setBrush( newBrush.brush );
	    dynamic_cast<KPPieObject*>( kpobject )->setLineBegin( newPen.lineBegin );
	    dynamic_cast<KPPieObject*>( kpobject )->setLineEnd( newPen.lineEnd );
	    dynamic_cast<KPPieObject*>( kpobject )->setFillType( newBrush.fillType );
	    dynamic_cast<KPPieObject*>( kpobject )->setGColor1( newBrush.gColor1 );
	    dynamic_cast<KPPieObject*>( kpobject )->setGColor2( newBrush.gColor2 );
	    dynamic_cast<KPPieObject*>( kpobject )->setGType( newBrush.gType );
	    dynamic_cast<KPPieObject*>( kpobject )->setGUnbalanced( newBrush.unbalanced );
	    dynamic_cast<KPPieObject*>( kpobject )->setGXFactor( newBrush.xfactor );
	    dynamic_cast<KPPieObject*>( kpobject )->setGYFactor( newBrush.yfactor );
	    doc->repaint( kpobject );
	    break;
	case OT_PART:
	    dynamic_cast<KPPartObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPPartObject*>( kpobject )->setBrush( newBrush.brush );
	    dynamic_cast<KPPartObject*>( kpobject )->setFillType( newBrush.fillType );
	    dynamic_cast<KPPartObject*>( kpobject )->setGColor1( newBrush.gColor1 );
	    dynamic_cast<KPPartObject*>( kpobject )->setGColor2( newBrush.gColor2 );
	    dynamic_cast<KPPartObject*>( kpobject )->setGType( newBrush.gType );
	    dynamic_cast<KPPartObject*>( kpobject )->setGUnbalanced( newBrush.unbalanced );
	    dynamic_cast<KPPartObject*>( kpobject )->setGXFactor( newBrush.xfactor );
	    dynamic_cast<KPPartObject*>( kpobject )->setGYFactor( newBrush.yfactor );
	    doc->repaint( kpobject );
	    break;
	case OT_TEXT:
	    dynamic_cast<KPTextObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPTextObject*>( kpobject )->setBrush( newBrush.brush );
	    dynamic_cast<KPTextObject*>( kpobject )->setFillType( newBrush.fillType );
	    dynamic_cast<KPTextObject*>( kpobject )->setGColor1( newBrush.gColor1 );
	    dynamic_cast<KPTextObject*>( kpobject )->setGColor2( newBrush.gColor2 );
	    dynamic_cast<KPTextObject*>( kpobject )->setGType( newBrush.gType );
	    dynamic_cast<KPTextObject*>( kpobject )->setGUnbalanced( newBrush.unbalanced );
	    dynamic_cast<KPTextObject*>( kpobject )->setGXFactor( newBrush.xfactor );
	    dynamic_cast<KPTextObject*>( kpobject )->setGYFactor( newBrush.yfactor );
	    doc->repaint( kpobject );
	    break;
	case OT_PICTURE:
	    dynamic_cast<KPPixmapObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPPixmapObject*>( kpobject )->setBrush( newBrush.brush );
	    dynamic_cast<KPPixmapObject*>( kpobject )->setFillType( newBrush.fillType );
	    dynamic_cast<KPPixmapObject*>( kpobject )->setGColor1( newBrush.gColor1 );
	    dynamic_cast<KPPixmapObject*>( kpobject )->setGColor2( newBrush.gColor2 );
	    dynamic_cast<KPPixmapObject*>( kpobject )->setGType( newBrush.gType );
	    dynamic_cast<KPPixmapObject*>( kpobject )->setGUnbalanced( newBrush.unbalanced );
	    dynamic_cast<KPPixmapObject*>( kpobject )->setGXFactor( newBrush.xfactor );
	    dynamic_cast<KPPixmapObject*>( kpobject )->setGYFactor( newBrush.yfactor );
	    doc->repaint( kpobject );
	    break;
	case OT_CLIPART:
	    dynamic_cast<KPClipartObject*>( kpobject )->setPen( newPen.pen );
	    dynamic_cast<KPClipartObject*>( kpobject )->setBrush( newBrush.brush );
	    dynamic_cast<KPClipartObject*>( kpobject )->setFillType( newBrush.fillType );
	    dynamic_cast<KPClipartObject*>( kpobject )->setGColor1( newBrush.gColor1 );
	    dynamic_cast<KPClipartObject*>( kpobject )->setGColor2( newBrush.gColor2 );
	    dynamic_cast<KPClipartObject*>( kpobject )->setGType( newBrush.gType );
	    dynamic_cast<KPClipartObject*>( kpobject )->setGUnbalanced( newBrush.unbalanced );
	    dynamic_cast<KPClipartObject*>( kpobject )->setGXFactor( newBrush.xfactor );
	    dynamic_cast<KPClipartObject*>( kpobject )->setGYFactor( newBrush.yfactor );
	    doc->repaint( kpobject );
	    break;
	default: break;
	}
    }

    newPen = tmpPen;
    newBrush = tmpBrush;
}

/*====================== unexecute ===============================*/
void PenBrushCmd::unexecute()
{
    KPObject *kpobject = 0L;

    for ( unsigned int i = 0; i < objects.count(); i++ ) {
	kpobject = objects.at( i );
	switch ( kpobject->getType() ) {
	case OT_LINE: {
	    if ( oldPen.count() > i ) {
		dynamic_cast<KPLineObject*>( kpobject )->setPen( oldPen.at( i )->pen );
		dynamic_cast<KPLineObject*>( kpobject )->setLineBegin( oldPen.at( i )->lineBegin );
		dynamic_cast<KPLineObject*>( kpobject )->setLineEnd( oldPen.at( i )->lineEnd );
		doc->repaint( kpobject );
	    }
	} break;
	case OT_RECT: {
	    if ( oldPen.count() > i )
		dynamic_cast<KPRectObject*>( kpobject )->setPen( oldPen.at( i )->pen );
	    if ( oldBrush.count() > i ) {
		dynamic_cast<KPRectObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
		dynamic_cast<KPRectObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
		dynamic_cast<KPRectObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
		dynamic_cast<KPRectObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
		dynamic_cast<KPRectObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
		dynamic_cast<KPRectObject*>( kpobject )->setGUnbalanced( oldBrush.at( i )->unbalanced );
		dynamic_cast<KPRectObject*>( kpobject )->setGXFactor( oldBrush.at( i )->xfactor );
		dynamic_cast<KPRectObject*>( kpobject )->setGYFactor( oldBrush.at( i )->yfactor );
	    }
	    doc->repaint( kpobject );
	} break;
	case OT_ELLIPSE: {
	    if ( oldPen.count() > i )
		dynamic_cast<KPEllipseObject*>( kpobject )->setPen( oldPen.at( i )->pen );
	    if ( oldBrush.count() > i ) {
		dynamic_cast<KPEllipseObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
		dynamic_cast<KPEllipseObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
		dynamic_cast<KPEllipseObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
		dynamic_cast<KPEllipseObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
		dynamic_cast<KPEllipseObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
		dynamic_cast<KPEllipseObject*>( kpobject )->setGUnbalanced( oldBrush.at( i )->unbalanced );
		dynamic_cast<KPEllipseObject*>( kpobject )->setGXFactor( oldBrush.at( i )->xfactor );
		dynamic_cast<KPEllipseObject*>( kpobject )->setGYFactor( oldBrush.at( i )->yfactor );
	    }
	    doc->repaint( kpobject );
	} break;
	case OT_AUTOFORM: {
	    if ( oldPen.count() > i )
		dynamic_cast<KPAutoformObject*>( kpobject )->setPen( oldPen.at( i )->pen );
	    if ( oldBrush.count() > i )
		dynamic_cast<KPAutoformObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
	    if ( oldPen.count() > i ) {
		dynamic_cast<KPAutoformObject*>( kpobject )->setLineBegin( oldPen.at( i )->lineBegin );
		dynamic_cast<KPAutoformObject*>( kpobject )->setLineEnd( oldPen.at( i )->lineEnd );
	    }
	    if ( oldBrush.count() > i ) {
		dynamic_cast<KPAutoformObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
		dynamic_cast<KPAutoformObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
		dynamic_cast<KPAutoformObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
		dynamic_cast<KPAutoformObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
		dynamic_cast<KPAutoformObject*>( kpobject )->setGUnbalanced( oldBrush.at( i )->unbalanced );
		dynamic_cast<KPAutoformObject*>( kpobject )->setGXFactor( oldBrush.at( i )->xfactor );
		dynamic_cast<KPAutoformObject*>( kpobject )->setGYFactor( oldBrush.at( i )->yfactor );
	    }
	    doc->repaint( kpobject );
	} break;
	case OT_PIE: {
	    if ( oldPen.count() > i )
		dynamic_cast<KPPieObject*>( kpobject )->setPen( oldPen.at( i )->pen );
	    if ( oldBrush.count() > i ) {
		dynamic_cast<KPPieObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
		dynamic_cast<KPPieObject*>( kpobject )->setLineBegin( oldPen.at( i )->lineBegin );
		dynamic_cast<KPPieObject*>( kpobject )->setLineEnd( oldPen.at( i )->lineEnd );
		dynamic_cast<KPPieObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
		dynamic_cast<KPPieObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
		dynamic_cast<KPPieObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
		dynamic_cast<KPPieObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
		dynamic_cast<KPPieObject*>( kpobject )->setGUnbalanced( oldBrush.at( i )->unbalanced );
		dynamic_cast<KPPieObject*>( kpobject )->setGXFactor( oldBrush.at( i )->xfactor );
		dynamic_cast<KPPieObject*>( kpobject )->setGYFactor( oldBrush.at( i )->yfactor );
	    }
	    doc->repaint( kpobject );
	} break;
	case OT_PART: {
	    if ( oldPen.count() > i )
		dynamic_cast<KPPartObject*>( kpobject )->setPen( oldPen.at( i )->pen );
	    if ( oldBrush.count() > i ) {
		dynamic_cast<KPPartObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
		dynamic_cast<KPPartObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
		dynamic_cast<KPPartObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
		dynamic_cast<KPPartObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
		dynamic_cast<KPPartObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
		dynamic_cast<KPPartObject*>( kpobject )->setGUnbalanced( oldBrush.at( i )->unbalanced );
		dynamic_cast<KPPartObject*>( kpobject )->setGXFactor( oldBrush.at( i )->xfactor );
		dynamic_cast<KPPartObject*>( kpobject )->setGYFactor( oldBrush.at( i )->yfactor );
	    }
	    doc->repaint( kpobject );
	} break;
	case OT_TEXT: {
	    if ( oldPen.count() > i )
		dynamic_cast<KPTextObject*>( kpobject )->setPen( oldPen.at( i )->pen );
	    if ( oldBrush.count() > i ) {
		dynamic_cast<KPTextObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
		dynamic_cast<KPTextObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
		dynamic_cast<KPTextObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
		dynamic_cast<KPTextObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
		dynamic_cast<KPTextObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
		dynamic_cast<KPTextObject*>( kpobject )->setGUnbalanced( oldBrush.at( i )->unbalanced );
		dynamic_cast<KPTextObject*>( kpobject )->setGXFactor( oldBrush.at( i )->xfactor );
		dynamic_cast<KPTextObject*>( kpobject )->setGYFactor( oldBrush.at( i )->yfactor );
	    }
	    doc->repaint( kpobject );
	} break;
	case OT_PICTURE: {
	    if ( oldPen.count() > i )
		dynamic_cast<KPPixmapObject*>( kpobject )->setPen( oldPen.at( i )->pen );
	    if ( oldBrush.count() > i ) {
		dynamic_cast<KPPixmapObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
		dynamic_cast<KPPixmapObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
		dynamic_cast<KPPixmapObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
		dynamic_cast<KPPixmapObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
		dynamic_cast<KPPixmapObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
		dynamic_cast<KPPixmapObject*>( kpobject )->setGUnbalanced( oldBrush.at( i )->unbalanced );
		dynamic_cast<KPPixmapObject*>( kpobject )->setGXFactor( oldBrush.at( i )->xfactor );
		dynamic_cast<KPPixmapObject*>( kpobject )->setGYFactor( oldBrush.at( i )->yfactor );
	    }
	    doc->repaint( kpobject );
	} break;
	case OT_CLIPART: {
	    if ( oldPen.count() > i )
		dynamic_cast<KPClipartObject*>( kpobject )->setPen( oldPen.at( i )->pen );
	    if ( oldBrush.count() > i ) {
		dynamic_cast<KPClipartObject*>( kpobject )->setBrush( oldBrush.at( i )->brush );
		dynamic_cast<KPClipartObject*>( kpobject )->setFillType( oldBrush.at( i )->fillType );
		dynamic_cast<KPClipartObject*>( kpobject )->setGColor1( oldBrush.at( i )->gColor1 );
		dynamic_cast<KPClipartObject*>( kpobject )->setGColor2( oldBrush.at( i )->gColor2 );
		dynamic_cast<KPClipartObject*>( kpobject )->setGType( oldBrush.at( i )->gType );
		dynamic_cast<KPClipartObject*>( kpobject )->setGUnbalanced( oldBrush.at( i )->unbalanced );
		dynamic_cast<KPClipartObject*>( kpobject )->setGXFactor( oldBrush.at( i )->xfactor );
		dynamic_cast<KPClipartObject*>( kpobject )->setGYFactor( oldBrush.at( i )->yfactor );
	    }
	    doc->repaint( kpobject );
	} break;
	default: break;
	}
    }
}


/******************************************************************/
/* Class: PgConfCmd                                               */
/******************************************************************/

/*================================================================*/
PgConfCmd::PgConfCmd( QString _name, bool _manualSwitch, bool _infinitLoop,
                      PageEffect _pageEffect, PresSpeed _presSpeed,
                      bool _oldManualSwitch, bool _oldInfinitLoop,
                      PageEffect _oldPageEffect, PresSpeed _oldPresSpeed,
                      KPresenterDoc *_doc, int _pgNum )
    : KCommand( _name )
{
    manualSwitch = _manualSwitch;
    infinitLoop = _infinitLoop;
    pageEffect = _pageEffect;
    presSpeed = _presSpeed;
    oldManualSwitch = _oldManualSwitch;
    oldInfinitLoop = _oldInfinitLoop;
    oldPageEffect = _oldPageEffect;
    oldPresSpeed = _oldPresSpeed;
    doc = _doc;
    pgNum = _pgNum;
}

/*================================================================*/
void PgConfCmd::execute()
{
    doc->setManualSwitch( manualSwitch );
    doc->setInfinitLoop( infinitLoop );
    doc->setPageEffect( pgNum, pageEffect );
    doc->setPresSpeed( presSpeed );
}

/*================================================================*/
void PgConfCmd::unexecute()
{
    doc->setManualSwitch( oldManualSwitch );
    doc->setInfinitLoop( oldInfinitLoop );
    doc->setPageEffect( pgNum, oldPageEffect );
    doc->setPresSpeed( oldPresSpeed );
}

/******************************************************************/
/* Class: PgLayoutCmd                                             */
/******************************************************************/

/*======================== constructor ===========================*/
PgLayoutCmd::PgLayoutCmd( QString _name, KoPageLayout _layout, KoPageLayout _oldLayout,
                          KPresenterView *_view )
    : KCommand( _name )
{
    layout = _layout;
    oldLayout = _oldLayout;
    view = _view;
}

/*====================== execute =================================*/
void PgLayoutCmd::execute()
{
    view->kPresenterDoc()->setPageLayout( layout, view->getDiffX(), view->getDiffY() );
    view->getHRuler()->setPageLayout( layout );
    view->getVRuler()->setPageLayout( layout );
    view->setRanges();
}

/*====================== unexecute ===============================*/
void PgLayoutCmd::unexecute()
{
    view->kPresenterDoc()->setPageLayout( oldLayout, view->getDiffX(), view->getDiffY() );
    view->getHRuler()->setPageLayout( oldLayout );
    view->getVRuler()->setPageLayout( oldLayout );
    view->setRanges();
}


/******************************************************************/
/* Class: PieValueCmd                                             */
/******************************************************************/

/*======================== constructor ===========================*/
PieValueCmd::PieValueCmd( QString _name, QPtrList<PieValues> &_oldValues, PieValues _newValues,
                          QPtrList<KPObject> &_objects, KPresenterDoc *_doc )
    : KCommand( _name ), oldValues( _oldValues ), objects( _objects )
{
    objects.setAutoDelete( false );
    oldValues.setAutoDelete( false );
    doc = _doc;
    newValues = _newValues;

    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->incCmdRef();
}

/*======================== destructor ============================*/
PieValueCmd::~PieValueCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->decCmdRef();
    oldValues.setAutoDelete( true );
    oldValues.clear();
}

/*====================== execute =================================*/
void PieValueCmd::execute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
    {
        dynamic_cast<KPPieObject*>( objects.at( i ) )->setPieType( newValues.pieType );
        dynamic_cast<KPPieObject*>( objects.at( i ) )->setPieAngle( newValues.pieAngle );
        dynamic_cast<KPPieObject*>( objects.at( i ) )->setPieLength( newValues.pieLength );
    }
    doc->repaint( false );
}

/*====================== unexecute ===============================*/
void PieValueCmd::unexecute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
    {
        dynamic_cast<KPPieObject*>( objects.at( i ) )->setPieType( oldValues.at( i )->pieType );
        dynamic_cast<KPPieObject*>( objects.at( i ) )->setPieAngle( oldValues.at( i )->pieAngle );
        dynamic_cast<KPPieObject*>( objects.at( i ) )->setPieLength( oldValues.at( i )->pieLength );
    }
    doc->repaint( false );
}


/******************************************************************/
/* Class: RectValueCmd                                            */
/******************************************************************/

/*======================== constructor ===========================*/
RectValueCmd::RectValueCmd( QString _name, QPtrList<RectValues> &_oldValues, RectValues _newValues,
                            QPtrList<KPObject> &_objects, KPresenterDoc *_doc )
    : KCommand( _name ), oldValues( _oldValues ), objects( _objects )
{
    objects.setAutoDelete( false );
    oldValues.setAutoDelete( false );
    doc = _doc;
    newValues = _newValues;

    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->incCmdRef();
}

/*======================== destructor ============================*/
RectValueCmd::~RectValueCmd()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        objects.at( i )->decCmdRef();
    oldValues.setAutoDelete( true );
    oldValues.clear();
}

/*====================== execute =================================*/
void RectValueCmd::execute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        dynamic_cast<KPRectObject*>( objects.at( i ) )->setRnds( newValues.xRnd, newValues.yRnd );

    doc->repaint( false );
}

/*====================== unexecute ===============================*/
void RectValueCmd::unexecute()
{
    for ( unsigned int i = 0; i < objects.count(); i++ )
        dynamic_cast<KPRectObject*>( objects.at( i ) )->setRnds( oldValues.at( i )->xRnd, oldValues.at( i )->yRnd );

    doc->repaint( false );
}

/******************************************************************/
/* Class: ResizeCmd						  */
/******************************************************************/

/*======================== constructor ===========================*/
ResizeCmd::ResizeCmd( QString _name, QPoint _m_diff, QSize _r_diff, KPObject *_object, KPresenterDoc *_doc )
    : KCommand( _name ), m_diff( _m_diff ), r_diff( _r_diff )
{
    object = _object;
    doc = _doc;
    object->incCmdRef();
}

/*======================== destructor ============================*/
ResizeCmd::~ResizeCmd()
{
    object->decCmdRef();
}

/*====================== execute =================================*/
void ResizeCmd::execute()
{
    QRect oldRect;

    oldRect = object->getBoundingRect( 0, 0 );
    object->moveBy( m_diff );
    object->resizeBy( r_diff );
    if ( object->getType() == OT_TEXT )
	( (KPTextObject*)object )->recalcPageNum( doc );
    doc->repaint( oldRect );
    doc->repaint( object );
}

/*====================== unexecute ===============================*/
void ResizeCmd::unexecute()
{
    unexecute(true);
}

/*====================== unexecute ===============================*/
void ResizeCmd::unexecute( bool _repaint )
{
    QRect oldRect;

    oldRect = object->getBoundingRect( 0, 0 );
    object->moveBy( -m_diff.x(), -m_diff.y() );
    object->resizeBy( -r_diff.width(), -r_diff.height() );
    if ( object->getType() == OT_TEXT )
	( (KPTextObject*)object )->recalcPageNum( doc );

    if ( _repaint ) {
	doc->repaint( oldRect );
	doc->repaint( object );
    }
}
