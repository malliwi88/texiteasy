/***************************************************************************
 *   copyright       : (C) 2013 by Quentin BRAMAS                          *
 *   http://texiteasy.com                                                  *
 *                                                                         *
 *   This file is part of texiteasy.                                          *
 *                                                                         *
 *   texiteasy is free software: you can redistribute it and/or modify        *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   texiteasy is distributed in the hope that it will be useful,             *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with texiteasy.  If not, see <http://www.gnu.org/licenses/>.       *                         *
 *                                                                         *
 ***************************************************************************/

#include "blockdata.h"
#include <QDebug>
BlockData::BlockData(int length)
{
    _length = length;
    misspelled = new bool[length];
    state = new char[length];
    parenthesisLevel.push(0);
    crocherLevel.push(0);
    for(int i = 0; i < length; ++i)
    {
        misspelled[i] = false;
    }
    for(int i = 0; i < length; ++i)
    {
        state[i] = 0;
    }
}
BlockData::~BlockData()
{
    delete[] misspelled;
    delete[] state;
}

QVector<ParenthesisInfo *> BlockData::parentheses() {
    return _parentheses;
}
QVector<LatexBlockInfo *> BlockData::latexblocks() {
    return _latexblocks;
}

void BlockData::insertPar( ParenthesisInfo *info ) {
    int i = 0;
    while (
        i < _parentheses.size() &&
        info->position > _parentheses.at(i)->position )
        ++i;
    _parentheses.insert( i, info );
}

void BlockData::insertLat( LatexBlockInfo *info ) {
    int i = 0;
    while (
        i < _latexblocks.size() &&
        info->position > _latexblocks.at(i)->position )
        ++i;
    _latexblocks.insert( i, info );
}

bool BlockData::isAClosingDollar(int position)
{
    bool even = false;
    foreach(int dollar, this->_dollars)
    {
        if(dollar > position)
        {
            return even;
        }
        even = !even;
    }
    return even;
}

BlockStartingState::BlockStartingState(BlockData *data, int state, int stateAfterOption)
{
    this->state = state;
    this->stateAfterOption = stateAfterOption;
    this->parenthesisLevel = data->parenthesisLevel;
    this->crocherLevel = data->crocherLevel;
}
bool BlockStartingState::equals(const BlockStartingState & other) const
{
    if(state != other.state)
    {
        return false;
    }
    if(stateAfterOption != other.stateAfterOption)
    {
        return false;
    }
    if(parenthesisLevel.count() != other.parenthesisLevel.count())
    {
        return false;
    }
    if(crocherLevel.count() != other.crocherLevel.count())
    {
        return false;
    }
    for(int idx = 0; idx < parenthesisLevel.count(); ++idx)
    {
        if(parenthesisLevel.at(idx) != other.parenthesisLevel.at(idx))
        {
            return false;
        }
    }
    for(int idx = 0; idx < crocherLevel.count(); ++idx)
    {
        if(crocherLevel.at(idx) != other.crocherLevel.at(idx))
        {
            return false;
        }
    }
    return true;
}
