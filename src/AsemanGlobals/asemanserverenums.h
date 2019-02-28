/*
    Copyright (C) 2017 Aseman Team
    http://aseman.co

    TelegramStats is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TelegramStats is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASEMANSERVERENUMS_H
#define ASEMANSERVERENUMS_H

namespace AsemanServerEnums {
    enum ParallelOperators {
        ParallelOperatorSubmit = 0xa1469aa,
        ParallelOperatorResult = 0x1f98aea,
        ParallelOperatorExitCore = 0x3e738a4,
        ParallelOperatorRequestNonParallel = 0x4947a47,
        ParallelOperatorBlockRequest = 0x5e8967d,
        ParallelOperatorTrustRequest = 0x408aa80,
        ParallelOperatorLog = 0x225f73a,
        ParallelOperatorReloadConfigRequest = 0x4dc7efb
    };

    enum SoftTypes {
        SoftTypeUnknown = 0,
        SoftTypeMethod = 0x1a6b431c,
        SoftTypeMethodResult = 0xde0a7e09,
        SoftTypeError = 0x1babf512,
        SoftTypeSignal = 0x8da8d7b9
    };

    enum HardOperators {
        HardOprUnknown = 0,
        HardOprTrustRequest = 0xadf91f3,
        HardOprTrustResult = 0x96dbec7,
        HardOprTrustUsingKey = 0x72f17a9,
        HardOprServiceCall = 0xe0a3723,
        HardOprServiceSignal = 0x71a3c3f,
        HardOprHardPing = 0x8f98c40,
        HardOprError = 0xbd9de66
    };

    enum HardOprErrors {
        HardOprErrUnknown = 0,
        HardOprErrTrustNeeded = 0xa1584a4,
        HardOprErrBadCommand = 0x1c8d8bd,
        HardOprErrBadStructure = 0xd54fcca,
        HardOprErrRejectAllCalls = 0xd844e3f,
        HardOprErrHostBlocked = 0x18ef937,
        HardOprErrBadServiceRequest = 0x3dd89d,
        HardOprErrDynamicError = 0xa415bf,
        HardOprErrLimitReached = 0x68772c
    };

    enum TrustQuestionType {
        TrustQstTypeBinary = 0x7c19293,
        TrustQstTypeJavaScript = 0x4e20d6a,
        TrustQstTypeNoNeed = 0x9cc3139
    };
}

#endif // ASEMANSERVERENUMS_H
