/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VSINUSTOOL_H__
#define __VSINUSTOOL_H__

#include "vshapetool.h"


class VSinusDlg;


class VSinusTool : public VShapeTool
{
public:
	VSinusTool( KarbonView* view );
	virtual ~VSinusTool();

	virtual void activate();

	virtual VPath* shape( bool decide = false ) const;

	virtual void showDialog() const;

private:
	VSinusDlg* m_dialog;
};

#endif

