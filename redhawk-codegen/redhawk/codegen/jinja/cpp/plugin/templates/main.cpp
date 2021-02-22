/*#
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 #*/

#include "${component.name}.h"
#include "struct_props.h"

static ${component.name}* main_plugin = 0;
static void sigint_handler(int signum)
{
    main_plugin->halt();
}

int main(int argc, char* argv[])
{
    struct sigaction sa;
    sa.sa_handler = &sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if( sigaction( SIGINT, &sa, NULL ) == -1 ) {
        exit(EXIT_FAILURE);
    }

    // Associate SIGQUIT to signal_catcher interrupt handler
    if( sigaction( SIGQUIT, &sa, NULL ) == -1 ) {
        exit(EXIT_FAILURE);
    }

    // Associate SIGTERM to signal_catcher interrupt handler
    if( sigaction( SIGTERM, &sa, NULL ) == -1 ) {
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, SIG_IGN);

    ossie::corba::CorbaInit(argc, argv);

    std::string ior;
    std::string id;
    if (argc < 3) {
        MessageConsumerPort* _consumer = new MessageConsumerPort("consumer");
        ior = ::ossie::corba::objectToString(_consumer->_this());
        id = "the id";
    } else {
        ior = argv[1];
        id = argv[2];
    }

    ${component.name}* my_plugin = new ${component.name}(ior, id);
    main_plugin = my_plugin;
    my_plugin->run();
    delete my_plugin;

    ossie::corba::OrbShutdown(true);
}
