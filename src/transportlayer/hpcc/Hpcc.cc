//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Hpcc.h"
#include "HpccSendQueue.h"

namespace inet {
namespace tcp {

Define_Module(Hpcc);

Hpcc::Hpcc() {
    // TODO Auto-generated constructor stub

}

Hpcc::~Hpcc() {
    // TODO Auto-generated destructor stub
}

TcpConnection* Hpcc::createConnection(int socketId)
{
    auto moduleType = cModuleType::get("hpcc.transportlayer.hpcc.HpccConnection");
    char submoduleName[24];
    sprintf(submoduleName, "conn-%d", socketId);
    auto module = check_and_cast<TcpConnection*>(moduleType->createScheduleInit(submoduleName, this));
    module->initConnection(this, socketId);
    return module;
}

TcpSendQueue *Hpcc::createSendQueue()
{
    return new HpccSendQueue();
}

}
}
