/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2021 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

#include "bitpit_common.hpp"

#include "piercedSync.hpp"

namespace bitpit {

/**
* \class PiercedSyncAction
* \ingroup containers
*
* \brief Action for pierced synchronization.
*/

/**
* Constructor
*/
PiercedSyncAction::PiercedSyncAction(ActionType _type)
    : type(_type), data(nullptr)
{
}

/**
* Copy constructor
*/
PiercedSyncAction::PiercedSyncAction(const PiercedSyncAction &other)
{
    // Copy the items that need allocation
    std::unique_ptr<std::vector<std::size_t>> new_data;
    if (other.data) {
        new_data = std::unique_ptr<std::vector<std::size_t>>(new std::vector<std::size_t>(*(other.data)));
    }

    // Assign the new memory to the object
    type = other.type;

    for (std::size_t k = 0; k < INFO_COUNT; ++k) {
        info[k] = other.info[k];
    }

    data.swap(new_data);
}

/*!
* Copy assignment operator.
*
* Assigns new contents to the object, replacing its current contents,
* and modifying its size accordingly.
*/
PiercedSyncAction & PiercedSyncAction::operator=(const PiercedSyncAction &other)
{
    if (this != &other) {
        PiercedSyncAction temporary(other);
        temporary.swap(*this);
    }

    return *this;
}

/*!
* Swaps the contents.
*
* \param other is another object of the same type
*/
void PiercedSyncAction::swap(PiercedSyncAction &other) noexcept
{
    std::swap(type, other.type);
    std::swap(info, other.info);
    data.swap(other.data);
}

/*!
* Import the given data.
*
* \param values are the values that will be imported
*/
void PiercedSyncAction::importData(const std::vector<std::size_t> &values)
{
    data = std::unique_ptr<std::vector<std::size_t>>(new std::vector<std::size_t>(values));
}

/**
* Restore the action.
*
* \param stream is the stream data should be read from
*/
void PiercedSyncAction::restore(std::istream &stream)
{
    int actionType;
    utils::binary::read(stream, actionType);
    type = static_cast<ActionType>(actionType);

    for (int k = 0; k < INFO_COUNT; ++k) {
        utils::binary::read(stream, info[k]);
    }

    std::size_t dataSize;
    utils::binary::read(stream, dataSize);
    data = std::unique_ptr<std::vector<std::size_t>>(new std::vector<std::size_t>(dataSize));
    for (std::size_t k = 0; k < dataSize; ++k) {
        utils::binary::read(stream, (*data)[k]);
    }
}

/**
* Dump the action.
*
* \param stream is the stream data should be written to
*/
void PiercedSyncAction::dump(std::ostream &stream) const
{
    utils::binary::write(stream, static_cast<int>(type));

    for (int k = 0; k < INFO_COUNT; ++k) {
        utils::binary::write(stream, info[k]);
    }

    std::size_t dataSize = data->size();
    utils::binary::write(stream, dataSize);
    for (std::size_t k = 0; k < dataSize; ++k) {
        utils::binary::write(stream, (*data)[k]);
    }
}

/**
* \class PiercedSyncSlave
* \ingroup containers
*
* \brief Base class for defining an object that acts like a slave in pierced
* synchronization.
*/

/**
* Constructor
*/
PiercedSyncSlave::PiercedSyncSlave()
{
}

/**
* Exchanges the content of the kernel by the content of x, which is another
* kernel object of the same type. Sizes may differ.
*
* After the call to this member function, the elements in this kernel are
* those which were in x before the call, and the elements of x are those
* which were in this. All iterators, references and pointers remain valid
* for the swapped objects.
*
* \param other is another kernel of the same type (i.e., instantiated with the
* same template parameters) whose content is swapped with that of this kernel.
*/
void PiercedSyncSlave::swap(PiercedSyncSlave &other) noexcept
{
    BITPIT_UNUSED(other);
}

/**
* \class PiercedSyncMaster
* \ingroup containers
*
* \brief Base class for defining an object that acts like a master in pierced
* synchronization.
*/

/**
* Constructor
*/
PiercedSyncMaster::PiercedSyncMaster()
    : m_syncEnabled(false)
{
    for (int k = SYNC_MODE_ITR_BEGIN; k != SYNC_MODE_ITR_END; ++k) {
        SyncMode mode = static_cast<SyncMode>(k);
        m_syncGroups[mode] = SyncGroup();
    }
}

/**
* Exchanges the content of the kernel by the content of x, which is another
* kernel object of the same type. Sizes may differ.
*
* After the call to this member function, the elements in this kernel are
* those which were in x before the call, and the elements of x are those
* which were in this. All iterators, references and pointers remain valid
* for the swapped objects.
*
* \param other is another kernel of the same type (i.e., instantiated with the
* same template parameters) whose content is swapped with that of this kernel.
*/
void PiercedSyncMaster::swap(PiercedSyncMaster &other) noexcept
{
    std::swap(other.m_slaves, m_slaves);
    std::swap(other.m_syncGroups, m_syncGroups);
    std::swap(other.m_syncJournal, m_syncJournal);
}

/**
* Register the specified slave
*
* \param slave is the slave that will be registered
* \param syncMode is the synchronization mode that will be used for the slave
*/
void PiercedSyncMaster::registerSlave(PiercedSyncSlave *slave, SyncMode syncMode) const
{
    if (m_slaves.count(slave) > 0) {
        throw std::out_of_range("Slave is already registered");
    }

    // Register the slave
    m_slaves.insert({slave, syncMode});

    // Add the slave to the proper synchronization group
    SyncGroup &syncGroup = m_syncGroups.at(syncMode);
    syncGroup.push_back(slave);

    // If needed enable synchronization
    if (!isSyncEnabled() && syncMode != SYNC_MODE_DISABLED) {
        setSyncEnabled(true);
    }
}

/**
* Process the specified synchronization action.
*
* \param action is the synchronization action that will be processed
*/
void PiercedSyncMaster::processSyncAction(const PiercedSyncAction &action)
{
    if (!isSyncEnabled()) {
        return;
    }

    // Synchronize concurrent slaves
    for (PiercedSyncSlave *slave : m_syncGroups.at(PiercedSyncMaster::SYNC_MODE_CONCURRENT)) {
        commitSyncAction(slave, action);
    }

    // If there are journaled slaves the synchronization action needs to be
    // journaled
    if (m_syncGroups.at(PiercedSyncMaster::SYNC_MODE_JOURNALED).size() > 0) {
        journalSyncAction(action);
    }
}

/**
* Commit the specified synchronization action to the specified slave->
*
* \param slave is the slave thr action will be commited to
* \param action is the synchronization action that will be commited
*/
void PiercedSyncMaster::commitSyncAction(PiercedSyncSlave *slave, const PiercedSyncAction &action) const
{
    slave->commitSyncAction(action);
}

/**
* Journal the specified synchronization action.
*
* \param action is the synchronization action that will be journaled
*/
void PiercedSyncMaster::journalSyncAction(const PiercedSyncAction &action)
{
    // Get information about previous action
    PiercedSyncAction *previousAction = nullptr;
    if (!m_syncJournal.empty()) {
        previousAction = &(m_syncJournal.back());
    }

    PiercedSyncAction::ActionType previousActionType = PiercedSyncAction::TYPE_UNDEFINED;
    if (previousAction) {
        previousActionType = previousAction->type;
    }

    // Journal current action
    //
    // If possible, current action will be merged with the previous one.
    switch (action.type) {

    case PiercedSyncAction::TYPE_CLEAR:
        m_syncJournal.resize(1);
        m_syncJournal[0] = action;
        break;

    case PiercedSyncAction::TYPE_APPEND:
        if (previousActionType == PiercedSyncAction::TYPE_APPEND) {
            previousAction->type = PiercedSyncAction::TYPE_RESIZE;
            previousAction->info[PiercedSyncAction::INFO_SIZE] = action.info[PiercedSyncAction::INFO_POS] + 1;
            previousAction->info[PiercedSyncAction::INFO_POS]  = std::numeric_limits<std::size_t>::max();
        } else if (previousActionType == PiercedSyncAction::TYPE_RESIZE) {
            previousAction->info[PiercedSyncAction::INFO_SIZE] = action.info[PiercedSyncAction::INFO_POS] + 1;
        } else {
            m_syncJournal.push_back(action);
        }
        break;

    case PiercedSyncAction::TYPE_RESIZE:
        if (previousActionType == PiercedSyncAction::TYPE_APPEND) {
            previousAction->type = PiercedSyncAction::TYPE_RESIZE;
            previousAction->info[PiercedSyncAction::INFO_SIZE] = action.info[PiercedSyncAction::INFO_SIZE];
            previousAction->info[PiercedSyncAction::INFO_POS]  = std::numeric_limits<std::size_t>::max();
        } else if (previousActionType == PiercedSyncAction::TYPE_RESIZE) {
            previousAction->info[PiercedSyncAction::INFO_SIZE] = action.info[PiercedSyncAction::INFO_SIZE];
        } else {
            m_syncJournal.push_back(action);
        }
        break;

    case PiercedSyncAction::TYPE_PIERCE:
        if (previousActionType == PiercedSyncAction::TYPE_PIERCE) {
            previousAction->type = PiercedSyncAction::TYPE_PIERCE_MULTIPLE;
            previousAction->data = std::unique_ptr<std::vector<std::size_t>>(new std::vector<std::size_t>(2));
            (*(previousAction->data))[0] = previousAction->info[PiercedSyncAction::INFO_POS];
            (*(previousAction->data))[1] = action.info[PiercedSyncAction::INFO_POS];
            previousAction->info[PiercedSyncAction::INFO_POS]      = std::numeric_limits<std::size_t>::max();
            previousAction->info[PiercedSyncAction::INFO_POS_NEXT] = std::numeric_limits<std::size_t>::max();
        } else if (previousActionType == PiercedSyncAction::TYPE_PIERCE_MULTIPLE) {
            previousAction->data->push_back(action.info[PiercedSyncAction::INFO_POS]);
        } else {
            m_syncJournal.push_back(action);
        }
        break;

    case PiercedSyncAction::TYPE_PIERCE_MULTIPLE:
        if (previousActionType == PiercedSyncAction::TYPE_PIERCE) {
            previousAction->type = PiercedSyncAction::TYPE_PIERCE_MULTIPLE;
            previousAction->data = std::unique_ptr<std::vector<std::size_t>>(new std::vector<std::size_t>(action.data->size() + 1));
            (*(previousAction->data))[0] = previousAction->info[PiercedSyncAction::INFO_POS];
            previousAction->data->insert(previousAction->data->begin() + 1, action.data->begin(), action.data->end());
            previousAction->info[PiercedSyncAction::INFO_POS]      = std::numeric_limits<std::size_t>::max();
            previousAction->info[PiercedSyncAction::INFO_POS_NEXT] = std::numeric_limits<std::size_t>::max();
        } else if (previousActionType == PiercedSyncAction::TYPE_PIERCE_MULTIPLE) {
            previousAction->data->insert(previousAction->data->begin(), action.data->begin(), action.data->end());
        } else {
            m_syncJournal.push_back(action);
        }
        break;

    case PiercedSyncAction::TYPE_OVERWRITE:
        if (previousActionType == PiercedSyncAction::TYPE_OVERWRITE) {
            previousAction->type = PiercedSyncAction::TYPE_OVERWRITE_MULTIPLE;
            previousAction->data = std::unique_ptr<std::vector<std::size_t>>(new std::vector<std::size_t>(2));
            (*(previousAction->data))[0] = previousAction->info[PiercedSyncAction::INFO_POS];
            (*(previousAction->data))[1] = action.info[PiercedSyncAction::INFO_POS];
            previousAction->info[PiercedSyncAction::INFO_POS] = std::numeric_limits<std::size_t>::max();
        } else if (previousActionType == PiercedSyncAction::TYPE_OVERWRITE_MULTIPLE) {
            previousAction->data->push_back(action.info[PiercedSyncAction::INFO_POS]);
        } else {
            m_syncJournal.push_back(action);
        }
        break;

    case PiercedSyncAction::TYPE_OVERWRITE_MULTIPLE:
        if (previousActionType == PiercedSyncAction::TYPE_OVERWRITE) {
            previousAction->type = PiercedSyncAction::TYPE_OVERWRITE_MULTIPLE;
            previousAction->data = std::unique_ptr<std::vector<std::size_t>>(new std::vector<std::size_t>(action.data->size() + 1));
            (*(previousAction->data))[0] = previousAction->info[PiercedSyncAction::INFO_POS];
            previousAction->data->insert(previousAction->data->begin() + 1, action.data->begin(), action.data->end());
            previousAction->info[PiercedSyncAction::INFO_POS] = std::numeric_limits<std::size_t>::max();
        } else if (previousActionType == PiercedSyncAction::TYPE_OVERWRITE_MULTIPLE) {
            previousAction->data->insert(previousAction->data->begin(), action.data->begin(), action.data->end());
        } else {
            m_syncJournal.push_back(action);
        }
        break;

    case PiercedSyncAction::TYPE_NOOP:
        break;

    default:
        m_syncJournal.push_back(action);
        break;

    }
}

/**
* Unregister the specified slave
*
* \param slave is the slave that will be unregistered
*/
void PiercedSyncMaster::unregisterSlave(const PiercedSyncSlave *slave) const
{
    // Check if the slave was actually registered
    if (!isSlaveRegistered(slave)) {
        return;
    }

    // Remove the slave from the synchronization group
    SyncMode syncMode = getSlaveSyncMode(slave);
    SyncGroup &syncGroup = m_syncGroups.at(syncMode);
    for (auto itr = syncGroup.begin(); itr != syncGroup.end(); ++itr) {
        PiercedSyncSlave *item = *itr;
        if (item == slave) {
            syncGroup.erase(itr);
            break;
        }
    }

    // Unregister the slave
    m_slaves.erase(const_cast<PiercedSyncSlave *>(slave));
}

/**
* Check if te specified slave is registered.
*
* \param slave is the slave to check
*/
bool PiercedSyncMaster::isSlaveRegistered(const PiercedSyncSlave *slave) const
{
    return (m_slaves.count(const_cast<PiercedSyncSlave *>(slave)) > 0);
}

/**
* Get the synchronization mode for the specified slave
*
* \param slave is the slave for which the synchronization mode is requested
* \result The synchronization mode of the slave
*/
PiercedSyncMaster::SyncMode PiercedSyncMaster::getSlaveSyncMode(const PiercedSyncSlave *slave) const
{
    return m_slaves.at(const_cast<PiercedSyncSlave *>(slave));
}

/**
* Check if the specified slave is synced.
*
* If synchronization is disabled for the slave, the slaved will be always
* considered out-of-sync.
*
* \param slave is the slave for which the check has to be performed
* \result Returns true if the slave is synchronized, false otherwise.
*/
bool PiercedSyncMaster::isSlaveSynced(const PiercedSyncSlave *slave) const
{
    SyncMode syncMode = getSlaveSyncMode(slave);
    if (syncMode == SYNC_MODE_CONCURRENT) {
        return true;
    } else if (syncMode == SYNC_MODE_JOURNALED) {
        return m_syncJournal.empty();
    } else {
        return false;
    }
}

/**
* Syncronize all the registered slaves.
*/
void PiercedSyncMaster::sync()
{
    // Only journaled slaved need to be synchronized
    for (PiercedSyncSlave *slave : m_syncGroups.at(SYNC_MODE_JOURNALED)) {
        for (const PiercedSyncAction &action : m_syncJournal) {
            commitSyncAction(slave, action);
        }
    }

    // Clear the sync journal
    m_syncJournal.clear();
    m_syncJournal.shrink_to_fit();
}

/**
* Check if all the slaves are synced.
*
* \result Returns true if all slave are synchronized, false otherwise.
*/
bool PiercedSyncMaster::isSynced() const
{
    for (const PiercedSyncSlave *slave : m_syncGroups.at(SYNC_MODE_JOURNALED)) {
        if (!isSlaveSynced(slave)) {
            return false;
        }
    }

    return true;
}

/**
* Enabled the synchronization.
*
* \param enabled is set to true the synchronization will be enabled, otherwise
*  it will be disabled
*/
void PiercedSyncMaster::setSyncEnabled(bool enabled) const
{
    m_syncEnabled = enabled;
}

/**
* Checks if the synchronization is enabled.
*
* \result Returns true if the synchronization is be enabled, otherwise it
* returns false
*/
bool PiercedSyncMaster::isSyncEnabled() const
{
    return m_syncEnabled;
}

/**
* Restore the object.
*
* \param stream is the stream data should be read from
*/
void PiercedSyncMaster::restore(std::istream &stream)
{
    std::size_t journalSize;
    utils::binary::read(stream, journalSize);
    m_syncJournal.resize(journalSize);
    for (PiercedSyncAction &action : m_syncJournal) {
        action.restore(stream);
    }
}

/**
* Dump the object.
*
* \param stream is the stream data should be written to
*/
void PiercedSyncMaster::dump(std::ostream &stream) const
{
    std::size_t journalSize = m_syncJournal.size();
    utils::binary::write(stream, journalSize);
    for (const PiercedSyncAction &action : m_syncJournal) {
        action.dump(stream);
    }
}

}
