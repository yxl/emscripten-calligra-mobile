/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "Chord.h"
#include "Note.h"
#include "Staff.h"
#include "VoiceBar.h"
#include "Clef.h"

#include <QtCore/QList>

#include <climits>

namespace MusicCore {

namespace {
    struct Beam {
        Beam(Chord* chord) : beamStart(chord), beamEnd(chord), beamType(Chord::BeamFlag) {}
        Chord* beamStart;
        Chord* beamEnd;
        Chord::BeamType beamType;
    };
}

class Chord::Private {
public:
    Chord::Duration duration;
    int dots;
    QList<Note*> notes;
    Chord::StemDirection stemDirection;
    double stemLength;
    QList<Beam> beams;
};

static double calcStemLength(Chord::Duration duration)
{
    switch (duration) {
        case Chord::Breve:
        case Chord::Whole:
            return 0;
        case Chord::Half:
        case Chord::Quarter:
        case Chord::Eighth:
            return 3.5;
        case Chord::Sixteenth:
            return 4;
        case Chord::ThirtySecond:
            return 4.75;
        case Chord::SixtyFourth:
            return 5.5;
        case Chord::HundredTwentyEighth:
            return 6.25;
    }
    return 0;
}

Chord::Chord(Duration duration, int dots) : VoiceElement(), d(new Private)
{
    d->duration = duration;
    d->dots = dots;
    d->stemLength = calcStemLength(duration);
    d->stemDirection = StemUp;
    
    int baseLength = durationToTicks(duration);
    int length = baseLength;
    for (int i = 0; i < dots; i++) {
        length += baseLength >> (i+1);
    }
    setLength(length);
    setWidth(7 + (3 * dots + (dots ? 2 : 0)));
}

Chord::Chord(Staff* staff, Duration duration, int dots) : d(new Private)
{
    d->duration = duration;
    d->dots = dots;
    d->stemLength = calcStemLength(duration);
    d->stemDirection = StemUp;

    int baseLength = durationToTicks(duration);
    int length = baseLength;
    for (int i = 0; i < dots; i++) {
        length += baseLength >> (i+1);
    }
    setLength(length);
    setStaff(staff);
    setWidth(7 + (3 * dots + (dots ? 2 : 0)));
}

Chord::~Chord()
{
    Q_FOREACH(Note* n, d->notes) delete n;
    delete d;
}

double Chord::width() const
{
    double w = 7;
    
    int lastPitch = INT_MIN;
    bool hasConflict = false;
    foreach (Note* n, d->notes) {
        int pitch = n->pitch();
        if (pitch == lastPitch+1) {
            hasConflict = true;
            break;
        }
        lastPitch = pitch;
    }
    
    if (hasConflict) w += 6;
    
    if (d->dots) {
        w += 2 + 3*d->dots;
    }
    
    return w;
}

Chord::Duration Chord::duration() const
{
    return d->duration;
}

void Chord::setDuration(Duration duration)
{
    if (d->duration == duration) return;
    d->duration = duration;
    d->stemLength = calcStemLength(duration);
    int baseLength = durationToTicks(d->duration);
    int length = baseLength;
    for (int i = 0; i < d->dots; i++) {
        length += baseLength >> (i+1);
    }
    setLength(length);
    emit durationChanged(duration);
}

int Chord::dots() const
{
    return d->dots;
}

void Chord::setDots(int dots)
{
    if (d->dots == dots) return;
    d->dots = dots;
    int baseLength = durationToTicks(d->duration);
    int length = baseLength;
    for (int i = 0; i < dots; i++) {
        length += baseLength >> (i+1);
    }
    setLength(length);
    emit dotsChanged(dots);
}

int Chord::noteCount() const
{
    return d->notes.size();
}

Note* Chord::note(int index)
{
    Q_ASSERT( index >= 0 && index < noteCount() );
    return d->notes[index];
}

Note* Chord::addNote(Staff* staff, int pitch, int accidentals)
{
    Note *n = new Note(staff, pitch, accidentals);
    addNote(n);
    return n;
}

void Chord::addNote(Note* note)
{
    if (!staff()) setStaff(note->staff());
    for (int i = 0; i < d->notes.size(); i++) {
        if (d->notes[i]->pitch() > note->pitch()) {
            d->notes.insert(i, note);
            return;
        }
    }
    d->notes.append(note);
}

void Chord::removeNote(int index, bool deleteNote)
{
    Q_ASSERT( index >= 0 && index < noteCount() );
    Note* n = d->notes.takeAt(index);
    if (deleteNote) {
        delete n;
    }
}

void Chord::removeNote(Note* note, bool deleteNote)
{
    Q_ASSERT( note );
    int index = d->notes.indexOf(note);
    Q_ASSERT( index != -1 );
    removeNote(index, deleteNote);
}

int Chord::durationToTicks(Duration duration)
{
    switch (duration) {
        case HundredTwentyEighth: return Note128Length;
        case SixtyFourth:         return Note64Length;
        case ThirtySecond:        return Note32Length;
        case Sixteenth:           return Note16Length;
        case Eighth:              return Note8Length;
        case Quarter:             return QuarterLength;
        case Half:                return HalfLength;
        case Whole:               return WholeLength;
        case Breve:               return DoubleWholeLength;
    }
    return 0;
}

QString Chord::durationToString(Duration duration)
{
    switch (duration) {
        case HundredTwentyEighth:   return "128th";
        case SixtyFourth:           return "64th";
        case ThirtySecond:          return "32nd";
        case Sixteenth:             return "16th";
        case Eighth:                return "eighth";
        case Quarter:               return "quarter";
        case Half:                  return "half";
        case Whole:                 return "whole";
        case Breve:                 return "breve";
    }
    return "[unknown note length]";
}

double Chord::y() const
{
    if (d->notes.size() == 0) {
        return staff()->lineSpacing();
    }

    double top = 1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);

    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());

        Staff* s = n->staff();
        line--;
        double y = s->top() + line * s->lineSpacing() / 2;
        if (y < top) top = y;
    }
    if (staff()) top -= staff()->top();
    return top;
}

double Chord::height() const
{
    if (d->notes.size() == 0) {
        return staff()->lineSpacing() * 2;
    }

    double top = 1e9;
    double bottom = -1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);

    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());

        Staff* s = n->staff();
        line--;
        double y = s->top() + line * s->lineSpacing() / 2;
        if (y < top) top = y;
        line += 2;
        y = s->top() + line * s->lineSpacing() / 2;
        if (y > bottom) bottom = y;
    }
    if (staff()) {
        top -= staff()->top();
        bottom -= staff()->top();
    }
    return bottom - top;
}

double Chord::stemX(double xScale) const
{
    int lastPitch = INT_MIN;
    bool hasConflict = false;
    foreach (Note* n, d->notes) {
        int pitch = n->pitch();
        if (pitch == lastPitch+1) {
            hasConflict = true;
            break;
        }
        lastPitch = pitch;
    }
    if (hasConflict) {
        return x() * xScale + 6;
    } else {
        return x() * xScale + (d->stemDirection == StemUp ? 6 : 0);
    }
}

double Chord::topNoteY() const
{
    if (d->notes.size() == 0) {
        return staff()->lineSpacing() * 2 + staff()->top();
    }
    
    double top = 1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);
    
    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());
        
        Staff* s = n->staff();
        double y = s->top() + line * s->lineSpacing() / 2;
        if (y < top) top = y;
    }
    return top;
}

double Chord::bottomNoteY() const
{
    if (d->notes.size() == 0) {
        return staff()->lineSpacing() * 2 + staff()->top();
    }
    
    double bottom = -1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);
    
    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());
        
        Staff* s = n->staff();
        double y = s->top() + line * s->lineSpacing() / 2;
        if (y > bottom) bottom = y;
    }
    return bottom;
}

double Chord::stemEndY(double xScale) const
{
    if (d->notes.size() == 0) return staff()->center();
    
    if (beamType(0) == BeamContinue) {
        // in the middle of a beam, interpolate stem length from beam
        double sx = beamStart(0)->stemX(xScale), ex = beamEnd(0)->stemX(xScale);
        double sy = beamStart(0)->stemEndY(xScale), ey = beamEnd(0)->stemEndY(xScale);
        double dydx = (ey-sy) / (ex-sx);
        
        return (stemX(xScale) - sx) * dydx + sy;
    }
    
    Staff* topStaff = NULL;
    Staff* bottomStaff = NULL;
    double top = 1e9, bottom = -1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);

    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());
        
        Staff* s = n->staff();
        double y = s->top() + line * s->lineSpacing() / 2;
        if (y > bottom) {
            bottom = y;
            bottomStaff = s;
        }
        if (y < top) {
            top = y;
            topStaff = s;
        }
    }
    
    Q_ASSERT( topStaff );
    Q_ASSERT( bottomStaff );
    
    if (stemDirection() == StemUp) {
        return top - topStaff->lineSpacing() * stemLength();
    } else {
        return bottom + bottomStaff->lineSpacing() * stemLength();
    }
}

double Chord::beamDirection(double xScale) const
{
    if (beamType(0) == BeamStart || beamType(0) == BeamEnd || beamType(0) == BeamContinue) {
        double sx = beamStart(0)->stemX(xScale), ex = beamEnd(0)->stemX(xScale);
        double sy = beamStart(0)->stemEndY(xScale), ey = beamEnd(0)->stemEndY(xScale);
        double dydx = (ey-sy) / (ex-sx);        
        return dydx;
    } else {
        return 0;
    }
}

Chord::StemDirection Chord::stemDirection() const
{
    return d->stemDirection;
}

void Chord::setStemDirection(StemDirection direction)
{
    d->stemDirection = direction;
}

double Chord::stemLength() const
{
    return d->stemLength;
}

void Chord::setStemLength(double stemLength)
{
    d->stemLength = stemLength;
}

int Chord::beamCount() const
{
    switch (d->duration) {
        case HundredTwentyEighth:   return 5;
        case SixtyFourth:           return 4;
        case ThirtySecond:          return 3;
        case Sixteenth:             return 2;
        case Eighth:                return 1;
        default:                    return 0;
    }            
}

const Chord* Chord::beamStart(int index) const
{
    if (d->beams.size() <= index) return this;
    return d->beams[index].beamStart;
}

const Chord* Chord::beamEnd(int index) const
{
    if (d->beams.size() <= index) return this;
    return d->beams[index].beamEnd;
}

Chord::BeamType Chord::beamType(int index) const
{
    if (d->beams.size() <= index) return BeamFlag;
    return d->beams[index].beamType;
}

void Chord::setBeam(int index, Chord* beamStart, Chord* beamEnd, BeamType type)
{
    Q_ASSERT( index < beamCount() );
    while (d->beams.size() <= index) {
        d->beams.append(Beam(this));
    }
    d->beams[index].beamStart = beamStart;
    d->beams[index].beamEnd = beamEnd;
    if (beamStart == this && beamEnd == this) {
        if (type != BeamFlag && type != BeamForwardHook && type != BeamBackwardHook) type = BeamFlag;
        d->beams[index].beamType = type;
    } else if (beamStart == this) d->beams[index].beamType = BeamStart;
    else if (beamEnd == this) d->beams[index].beamType = BeamEnd;
    else d->beams[index].beamType = BeamContinue;
}

} // namespace MusicCore

#include "Chord.moc"
