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

package hpcc.simulations.ExperimentMultipath;

@namespace(inet);

import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import ned.DatarateChannel;
import ned.IBidirectionalChannel;
import inet.common.scenario.ScenarioManager;

network manhattan
{
    parameters:
        @display("bgb=700,500");
        int numberOfServers = default(1);
        int numberOfNormalFlows = default(1);
        int numberOfLongerFlows = default(0);
        int numberOfRouters = default(1);
    types:
        channel satHop extends DatarateChannel //look at 
        {
            delay = 1.5ms;
            datarate = 10Gbps;
        }
        channel bottleneckHop extends DatarateChannel //look at 
        {
            delay = 1.5ms;
            datarate = 1Gbps;
        }
        channel satHopLarge extends DatarateChannel //look at 
        {
            delay = 10.5ms;
            datarate = 10Gbps;
        }
    submodules:
        client1: StandardHost {
            @display("p=153,27");
        }
        client2: StandardHost {
            @display("p=100,75");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=475,-20");
        }
        server1: StandardHost {
            @display("p=616.896,452.592");
        }
        server2: StandardHost {
            @display("p=632.016,357.84");
        }
        router2: Router {
            @display("p=205,62");
        }
        router3: Router {
            @display("p=296,62");
        }
        router4: Router {
            @display("p=386,62");
        }
        router5: Router {
            @display("p=475,62");
        }
        router6: Router {
            @display("p=114,137");
        }
        router10: Router {
            @display("p=475,137");
        }
        router11: Router {
            @display("p=114,205");
        }
        router15: Router {
            @display("p=475,205");
        }
        router16: Router {
            @display("p=114,272");
        }
        router20: Router {
            @display("p=475,272");
        }
        router21: Router {
            @display("p=114,337");
        }
        router22: Router {
            @display("p=205,337");
        }
        router23: Router {
            @display("p=296,337");
        }
        router24: Router {
            @display("p=386,337");
        }
        router25: Router {
            @display("p=475,337");
        }
        router26: Router {
            @display("p=562.464,336.672");
        }
        client3: StandardHost {
            @display("p=386,414");
        }
        server3: StandardHost {
            @display("p=625.968,282.24002");
        }
    connections:
        client1.pppg++ <--> satHop <--> router2.pppg++;
        client2.pppg++ <--> satHop <--> router6.pppg++;
        client3.pppg++ <--> satHopLarge <--> router24.pppg++;
        router2.pppg++ <--> satHop <--> router3.pppg++;
        router3.pppg++ <--> satHop <--> router4.pppg++;
        router4.pppg++ <--> satHop <--> router5.pppg++;
        router21.pppg++ <--> satHop <--> router22.pppg++;
        router22.pppg++ <--> satHop <--> router23.pppg++;
        router23.pppg++ <--> satHop <--> router24.pppg++;
        router24.pppg++ <--> satHop <--> router25.pppg++;

        router6.pppg++ <--> satHop <--> router11.pppg++;
        router11.pppg++ <--> satHop <--> router16.pppg++;
        router16.pppg++ <--> satHop <--> router21.pppg++;

        router5.pppg++ <--> satHop <--> router10.pppg++;
        router10.pppg++ <--> satHop <--> router15.pppg++;
        router15.pppg++ <--> satHop <--> router20.pppg++;
        router20.pppg++ <--> satHop <--> router25.pppg++;

        router25.pppg++ <--> bottleneckHop <--> router26.pppg++;

        router26.pppg++ <--> satHop <--> server1.pppg++;
        router26.pppg++ <--> satHop <--> server2.pppg++;
        router26.pppg++ <--> satHop <--> server3.pppg++;
}
