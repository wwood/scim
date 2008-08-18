/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#define USE_SCIM_KDE_SETTINGS
#include "src/skimpluginmanager.h"
#include "scimlauncher.h"

#include <klocale.h>

using namespace scim;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <kapplication.h>
#include <kgenericfactory.h>
#include <kprocess.h>

#include <qtimer.h>

K_EXPORT_COMPONENT_FACTORY( skimplugin_scim,
                              KGenericFactory<ScimLauncher>( "skimplugin_scim" ) )

ScimThread::ScimThread(QString _m_args)
{
    m_args = "skim " + _m_args;
    m_new_argc = 0;
}

ScimThread::~ScimThread()
{
    for (int i = 0; i < m_new_argc; ++i) {
        free (m_new_argv[m_new_argc]);
    }
}

void ScimThread::run()
{
    BackEndPointer       backend;

    std::vector<String>  frontend_list;
    std::vector<String>  config_list;
    std::vector<String>  engine_list;
    std::vector<String>  exclude_engine_list;
    std::vector<String>  load_engine_list;

    String def_frontend;
    String def_config;

    size_t i;
    bool daemon = true;
    bool socket = true;
    bool manual = false;

    //get modules list
    scim_get_frontend_module_list (frontend_list);
    scim_get_imengine_module_list (engine_list);
    scim_get_config_module_list   (config_list);

    //Use x11 FrontEnd as default if available.
    if (frontend_list.size ()) {
        def_frontend = String ("x11");
        if (std::find (frontend_list.begin (),
                       frontend_list.end (),
                       def_frontend) == frontend_list.end ())
            def_frontend = frontend_list [0];
    }

    //Add a dummy config module, it's not really a module!
    config_list.push_back ("dummy");

    //Use kconfig Config module as default if available.
    def_config = "kconfig";
    if (config_list.size ()) {
        if (std::find (config_list.begin (),
                       config_list.end (),
                       def_config) == config_list.end ())
            def_config = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));
        if (std::find (config_list.begin (),
                       config_list.end (),
                       def_config) == config_list.end ())
            def_config = config_list [0];
    }

    // If no Socket Config/IMEngine/FrontEnd modules
    // then do not try to start a SocketFrontEnd.
    if (std::find (frontend_list.begin (), frontend_list.end (), "socket") == frontend_list.end () ||
        std::find (config_list.begin (), config_list.end (), "socket") == config_list.end () ||
        std::find (engine_list.begin (), engine_list.end (), "socket") == engine_list.end ())
        socket = false;

    //parse command options
    i = 0;
    QStringList argv = QStringList::split(' ', m_args);
    uint argc = argv.size();
    while (i<argc) {
        if (++i >= argc) break;

        if ("-l" == argv [i] ||
            "--list" == argv [i]) {
            std::vector<String>::iterator it;

            cout << endl;
            cout << "Available FrontEnd module:\n";
            for (it = frontend_list.begin (); it != frontend_list.end (); it++)
                cout << "    " << *it << endl;

            cout << endl;
            cout << "Available Config module:\n";
            for (it = config_list.begin (); it != config_list.end (); it++)
                cout << "    " << *it << endl;

            cout << endl;
            cout << "Available IMEngine module:\n";
            for (it = engine_list.begin (); it != engine_list.end (); it++)
                cout << "    " << *it << endl;

            return;
        }

        if ("-f" == argv [i] ||
            "--frontend" == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return;
            }
            def_frontend = argv [i].utf8();
            continue;
        }

        if ("-c" == argv [i] ||
            "--config" == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return;
            }
            def_config = argv [i].utf8();
            continue;
        }

        if ("-h" == argv [i] ||
            "--help" == argv [i]) {
            cout << "Usage: " << argv [0] << " [option]...\n\n"
                 << "The options are: \n"
                 << "  -l, --list              List all of available modules.\n"
                 << "  -f, --frontend name     Use specified FrontEnd module.\n"
                 << "  -c, --config name       Use specified Config module.\n"
                 << "  -e, --engines name      Load specified set of IMEngines.\n"
                 << "  -ne,--no-engines name   Do not load those set of IMEngines.\n"
                 << "  -d, --daemon            Run " << argv [0] << " as a daemon.\n"
                 << "  --no-socket             Do not try to start a SCIM SocketFrontEnd daemon.\n"
                 << "  -h, --help              Show this help message.\n";
            return;
        }

        if ("-d" == argv [i] ||
            "--daemon" == argv [i]) {
            daemon = true;
            continue;
        }

        if ("-e" == argv [i] || "-s" == argv [i] ||
            "--engines" == argv [i] || "--servers" == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return;
            }
            scim_split_string_list (load_engine_list, String (argv [i].utf8()), ',');
            manual = true;
            continue;
        }

        if ("-ne" == argv [i] || "-ns" == argv [i] ||
            "--no-engines" == argv [i] || "-no-servers" == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return;
            }
            scim_split_string_list (exclude_engine_list, String (argv [i].utf8()), ',');
            manual = true;
            continue;
        }

        if ("--no-socket" == argv [i]) {
            socket = false;
            continue;
        }

        if ("--" == argv [i])
            break;

        cerr << "Invalid command line option: " << argv [i] << "\n";
        return;
    } //End of command line parsing.

    // Store the rest argvs into m_new_argv.
    for (++i; i < argc; ++i) {
        m_new_argv [m_new_argc ++] = qstrdup(argv [i].utf8());
    }

    m_new_argv [m_new_argc] = 0;

    // Get the imengine module list which should be loaded.
    if (exclude_engine_list.size ()) {
        load_engine_list.clear ();
        for (i = 0; i < engine_list.size (); ++i) {
            if (std::find (exclude_engine_list.begin (),
                           exclude_engine_list.end (),
                           engine_list [i]) == exclude_engine_list.end () &&
                engine_list [i] != "socket")
                load_engine_list.push_back (engine_list [i]);
        }
    }

    if (!def_frontend.length ()) {
        cerr << "No FrontEnd module is available!\n";
        return;
    }

    if (!def_config.length ()) {
        cerr << "No Config module is available!\n";
        return;
    }

    // If you try to use the socket feature manually,
    // then let you do it by yourself.
    if (def_frontend == "socket" || def_config == "socket" ||
        std::find (load_engine_list.begin (), load_engine_list.end (), "socket") != load_engine_list.end ())
        socket = false;

    // If the socket address of SocketFrontEnd and SocketIMEngine/SocketConfig are different,
    // then do not try to start the SocketFrontEnd instance automatically.
    if (scim_get_default_socket_frontend_address () != scim_get_default_socket_imengine_address () ||
        scim_get_default_socket_frontend_address () != scim_get_default_socket_config_address ())
        socket = false;

    // Try to start a SCIM SocketFrontEnd daemon first.
    if (socket) {
        // If no Socket FrontEnd is running, then launch one.
        // And set manual to false.
        if (!check_socket_frontend ()) {
            cout << "Launching a SCIM daemon with Socket FrontEnd...\n";
            char no_stay[] = "--no-stay";
            char *no_stay_argv [] = {no_stay, 0 };
            scim_launch (true,
                         def_config,
                         (load_engine_list.size () ? scim_combine_string_list (load_engine_list, ',') : "all"),
                         "socket",
                         no_stay_argv);
            manual = false;
        }

      // If there is one Socket FrontEnd running and it's not manual mode,
      // then just use this Socket Frontend.
      if (!manual) {
            for (int i = 0; i < 100; ++i) {
                if (check_socket_frontend ()) {
                    def_config = "socket";
                    load_engine_list.clear ();
                    load_engine_list.push_back ("socket");
                    break;
                }
                scim_usleep (100000);
            }
      }
    }

    cout << "Launching a SCIM process with " << def_frontend << "...\n";

    // Launch the scim process.
    if (scim_launch (daemon,
                     def_config,
                     (load_engine_list.size () ? scim_combine_string_list (load_engine_list, ',') : "all"),
                     def_frontend,
                     m_new_argv) == 0) {
        if (daemon)
            cerr << "SCIM has been successfully launched.\n";
        else
            cerr << "SCIM has exited successfully.\n";

        return;
    }

    if (daemon)
        cerr << "Failed to launch SCIM.\n";
    else
        cerr << "SCIM has exited abnormally.\n";

    return;
}

bool ScimThread::check_socket_frontend ()
{
    scim::SocketAddress address;
    scim::SocketClient client;

    uint32 magic;

    address.set_address (scim_get_default_socket_frontend_address ());

    if (!client.connect (address))
        return false;

    if (!scim_socket_open_connection (magic,
                                            String ("ConnectionTester"),
                                            String ("SocketFrontEnd"),
                                            client,
                                            1000)) {
        return false;
    }

    return true;
}

ScimLauncher::ScimLauncher(QObject *parent, const char *name, const QStringList & /*m_args*/)
 : SkimPlugin(KGenericFactory<ScimLauncher>::instance(), parent, name)
{
    m_scimThread = new ScimThread("");
    m_scimThread->start();
}

void ScimLauncher::aboutToUnload() {
    if(!m_scimThread->wait(3000)) {
        std::cout << "ScimLauncher exited abnormally\n";
        m_scimThread->terminate();
    }
    
    SkimPlugin::aboutToUnload();
}

ScimLauncher::~ScimLauncher()
{
}


#include "scimlauncher.moc"
