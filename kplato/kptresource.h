/* This file is part of the KDE project
   Copyright (C) 2001 Thomas Zander zander@kde.org

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

#ifndef kptresource_h
#define kptresource_h

#include <qstring.h>
#include "kptduration.h"
#include <qlist.h>
#include "defs.h"

class KPTRisk;
class KPTEffort;
class KPTAppointment;
class KPTTask;
class KPTNode;
class KPTResource;

/**
  * This class represents a group of similar resources to be assigned to a task
  * e.g. The list of employees, computer resources, etc
  */

class KPTResourceGroup {
    public:
	      KPTResourceGroup();
	      ~KPTResourceGroup();
	
	      void setName(QString n) {m_name=n;}
	      const QString &name() const {return m_name;}
	
	      /** Manage the resources in this list
	        * At some point we will have to look at not mixing types of resources
	        * (e.g. you can't add a person to a list of computers
	        *
	        * Risks must always be associated with a resource, so there is no option
	        * to manipulate risks seperately
	        */
	
	      void addResource(KPTResource*, KPTRisk*);
        void insertResource( unsigned int index, KPTResource *resource );
        void removeResource( KPTResource *resource );
	      void removeResource(int);

	      KPTResource* getResource(int);
	      KPTRisk* getRisk(int);
 	
	      /** Manage the dependent resources.  This is a list of the resource
	        * groups that must have available resources for this resource to
	        * perform the work
	        */
	
	      void addRequiredResource(KPTResourceGroup*);
	      KPTResourceGroup* getRequiredResource(int);
	      void removeRequiredResource(int);
	
    private:
        QString m_name;
        QList<KPTResource> m_resources;
        QList<KPTRisk> m_risks;
        QList<KPTResourceGroup> m_requires;
};

class KPTResource {
    public:

        KPTResource();
        ~KPTResource();


        void setName(QString);
        QString &name();

        void setAvailableFrom(KPTDuration af) {m_availableFrom=af;}
        const KPTDuration &availableFrom() const {return m_availableFrom;}
        void setAvailableUntil(KPTDuration au) {m_availableUntil=au;}
        const KPTDuration &availableUntil() const {return m_availableUntil;}

        void addWorkingHour(KPTDuration from, KPTDuration until);

        KPTDuration *getFirstAvailableTime(KPTDuration after = KPTDuration());
        KPTDuration *getBestAvailableTime(KPTDuration duration);

    private:
        QList<KPTAppointment> m_appointments;
        QString m_name;
        KPTDuration m_availableFrom;
        KPTDuration m_availableUntil;
        QList<KPTDuration> m_workingHours;
};

class KPTAppointment {
    public:
        KPTAppointment(KPTDuration startTime, KPTDuration duration, KPTResource *resource, KPTTask *taskNode=0);
        ~KPTAppointment();

        // get/set member values.
        const KPTDuration &startTime() const { return m_startTime; }
        void setStartTime(KPTDuration st) { m_startTime=st; }

        const KPTDuration &duration() const { return m_duration; }
        void setDuration(KPTDuration d) { m_duration=d; }

        KPTTask *task() { return m_task; }
        void setTask(KPTTask *t) { m_task = t; }

        const KPTDuration &repeatInterval() const {return m_repeatInterval;}
        void setRepeatInterval(KPTDuration ri) {m_repeatInterval=ri;}

        int repeatCount() { return m_repeatCount; }
        void setRepeatCount(int rc) { m_repeatCount=rc; }

        void deleteAppointmentFromRepeatList(KPTDuration time);
        void addAppointmentToRepeatList(KPTDuration time);

    private:
        KPTDuration m_startTime;
        KPTDuration m_duration;
        KPTTask *m_task;
        KPTResource *m_resource;
        KPTDuration m_repeatInterval;
        int m_repeatCount;
        QList<KPTDuration> m_extraRepeats;
        QList<KPTDuration> m_skipRepeats;
};


/** 
 * Risk is associated with a resource/task pairing to indicate the planner's confidence in the 
 * estimated effort. Risk can be one of none, low, or high. Some factors that may be taken into 
 * account for risk are the experience of the person and the reliability of equipment.
 */
class KPTRisk {
    public:

        enum RiskType {
            NONE=0,
            LOW=1,
            HIGH=2
        };

        KPTRisk(KPTNode *n, KPTResource *r, RiskType rt=NONE);
        ~KPTRisk();

        RiskType riskType() { return m_riskType; }

        KPTNode *node() { return m_node; }
        KPTResource *resource() { return m_resource; }

    private:
        KPTNode *m_node;
        KPTResource *m_resource;
        RiskType m_riskType;
};

#endif
