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

package hpcc.simulations.ExperimentPathChange;

@namespace(inet);

import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import hpcc.networklayer.configurator.ipv4.Ipv4NetworkConfiguratorUpdate;
import inet.node.inet.Router;
import ned.DatarateChannel;
import ned.IBidirectionalChannel;
import inet.common.scenario.ScenarioManager;

network pathchange
{
    parameters:
        @display("bgb=700,500");
        int numberOfServers = default(1);
        int numberOfNormalFlows = default(1);
        int numberOfLongerFlows = default(0);
        int numberOfRouters = default(1);
    types:
        channel fastLink extends DatarateChannel //look at 
        {
            delay = 0ms;
            datarate = 10Gbps;
        }
        channel bottleneckHop1 extends DatarateChannel //look at 
        {
            delay = 5ms;
            datarate = 1Gbps;
        }
        channel bottleneckHop2 extends DatarateChannel //look at 
        {
            delay = 10ms;
            datarate = 1Gbps;
        }
        channel slowHop extends DatarateChannel //look at 
        {
            delay = 500ms;
            datarate = 1Gbps;
        }
    submodules:
        client1: StandardHost {
            @display("p=107.856,80.64");
        }
        client2: StandardHost {
            @display("p=107.856,169.34401");
        }
        configurator: Ipv4NetworkConfiguratorUpdate {
            @display("p=475,-20");
        }
        server1: StandardHost {
            @display("p=502.992,80.64");
        }
        server2: StandardHost {
            @display("p=502.992,169.34401");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,300;is=s");
        }
        router1: Router {
            @display("p=244.944,80.64");
        }
        router2: Router {
            @display("p=374.976,80.64");
        }
        router3: Router {
            @display("p=244.944,169.34401");
        }
        router4: Router {
            @display("p=374.976,169.34401");
        }
    connections:
        client1.pppg++ <--> fastLink <--> router1.pppg++;
        client2.pppg++ <--> fastLink <--> router3.pppg++;
        router2.pppg++ <--> fastLink <--> server1.pppg++;
        router2.pppg++ <--> fastLink <--> server2.pppg++;
        router4.pppg++ <--> fastLink <--> server2.pppg++;

        router1.pppg++ <--> bottleneckHop1 <--> router2.pppg++;
        router3.pppg++ <--> bottleneckHop2 <--> router4.pppg++;
        
        client2.pppg++ <--> slowHop <--> router1.pppg++;
}
