#ifndef ROOMS_H
#define ROOMS_H

// ---------------------------------------------------------
// 1. Room identifiers
// ---------------------------------------------------------
// These IDs are used by room logic, background loading,
// collision map selection, and the main state machine.
typedef enum
{
    ROOM_TITLESCREEN = 0,
    ROOM_ARCADE1,
    ROOM_ARCADE2,
    ROOM_BIOPOD,
    ROOM_HYDROPONICSBAY,
    ROOM_HANGARBAY,
    ROOM_TRAININGDECK,
    ROOM_REACTORCHAMBER,
    ROOM_SLEEPINGQUARTERS,
    ROOM_ARMORYVAULT,
    ROOM_HOLODECK,
    ROOM_MEDICALBAY,
    ROOM_SCIENCELAB,

    ROOM_COUNT
} RoomId;

#endif
