
#ifndef DATABASE_STATUS_HPP
#define DATABASE_STATUS_HPP

#include <core/status.hpp>

namespace Database
{

    enum class StatusCodes
    {
        Ok,
        BadVersion,                         // db format is unknown (newer that supported)
        OpenFailed,
        ProjectLocked,
        TransactionFailed,                  // Fail at transaction begin.
        TransactionCommitFailed,            // Fail at transaction commit
        QueryPreparationFailed,
        QueryFailed,
        MigrationFailed,
    };

    typedef Status<StatusCodes, StatusCodes::Ok> BackendStatus;

}

#endif
