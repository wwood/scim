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
#include "skimpluginmanager.h"

#include <kapplication.h> 
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
 
#include "utils/skimplugininfo.h"

#include <signal.h>

static const char description[] =
    I18N_NOOP("KDE Frontend for SCIM Input Method Platform");

static KCmdLineOptions options[] =
{
    { "d", I18N_NOOP( "Start as daemon." ), 0 },
    { "l", I18N_NOOP( "List all availabe skim plugins." ), 0 },
    { "c <name>", I18N_NOOP( "Uses specified Config module." ), 0 },
    { "p <enabled plugins>", I18N_NOOP( "A comma separated list of plugin names. All of these plugins will be loaded no matter whether they are enabled in the setting file." ), 0 },
    { "np <disabled plugins>", I18N_NOOP( "A comma separated list of plugin names. All of these plugins will be disabled no matter whether they are disabled in the setting file." ), 0 },
    { "no-stay", I18N_NOOP( "Quit if no connected client." ), "false" },
    { "verbose <level>", I18N_NOOP( " Enable debug info, to specific LEVEL.." ), "false" },
    { "V", I18N_NOOP( "Current version of skim" ), 0 },
    { "o <file>", I18N_NOOP( "Output debug file.(No effort, always use none)" ), 0 },
    { "f", I18N_NOOP( "Force start even if scim panel socket can not be established." ), 0 },
//     { "display <DISPLAY>", I18N_NOOP( "Run on display DISPLAY." ), 0 },
    KCmdLineLastOption
};

static void sighandler(int)
{
    fprintf(stderr, "skim: sighandler called\n");
    QApplication::exit(-1);
}

int main( int argc, char ** argv )
{
    //disable qt module support for skim itself; xim will be disabled latter
    setenv("QT_IM_SWITCHER", "imsw-none", 1);
    setenv("QT_IM_MODULE", "xim", 1);
    setenv("XMODIFIER", "@im=none", 1);

    KAboutData about(PACKAGE, "SKIM",
        VERSION " (compiled with libscim " SCIM_VERSION ")",
        description, KAboutData::License_GPL_V2, "(C) 2004 - 2006 LiuCougar",
        I18N_NOOP("IRC:\nserver: irc.freenode.net / channel: #scim\n\nFeedback:\nscim-user@lists.sourceforge.net"),
        "http://www.scim-im.org");
    about.addAuthor( "LiuCougar (liuspider)", I18N_NOOP("Core Developer"),
      "liuspider@users.sourceforge.net" );
    about.addCredit ( "JamesSu", I18N_NOOP("SCIM Core Author"), "suzhe@tsinghua.org.cn" );
    about.addAuthor ( "JanHefti", I18N_NOOP("Doc writer and German translator"),
      "j.hefti@hamburg.de" );
    about.addAuthor ( "KitaeKim", I18N_NOOP("Art designer and Korean translator"),
      "neeum@yahoo.com" );
    about.addAuthor ( "YukikoBando", I18N_NOOP("Japanese translator"),
      "ybando@k6.dion.ne.jp" );
    about.setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names")
        ,I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

    QString curarg;

    QStringList otherArgs;

    for(int i = 1; i < argc ; i++)
    {
        curarg = argv[i];
        if( curarg == "--no-stay")
            otherArgs.push_back("no-stay");
        else if (curarg == "-c" ) {
            otherArgs.push_back("c");
            //FIXME
            otherArgs.push_back(argv[++i]);
        } else if( curarg == "-f")
            otherArgs.push_back("force");
    }

    //FIXME: noxim is necessary to disable xim support in qt for this app
    const char* fake_arg1 =  "--noxim";
    char* fake_argv[10] = {argv[0], const_cast<char *>(fake_arg1), 0, 0, 0, 0, 0};
    for(int i = 1; i < argc; i++)
    {
      fake_argv[i+1] = argv[i];
    }
    KCmdLineArgs::init(argc+1, fake_argv, &about);

    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    scim::uint32 verbose_level = 0;
    QString verbose_raw = args->getOption("verbose");
    if( verbose_raw.length() && verbose_raw.toInt())
        verbose_level = verbose_raw.toInt();

    scim::DebugOutput::set_verbose_level( verbose_level );
    scim::DebugOutput::enable_debug (SCIM_DEBUG_AllMask);
//     scim::DebugOutput::enable_debug (SCIM_DEBUG_MainMask | SCIM_DEBUG_SocketMask);

    if( args->isSet("l") ) {
      new KInstance(PACKAGE);
      QValueList<SkimPluginInfo *> info = SkimPluginManager::allAvailablePlugins();
      std::cout << I18N_NOOP("Installed skim Plugins:") << "\n" << I18N_NOOP("Name") << "\t\t\t\t" << I18N_NOOP("Comment") << "\n";
      for(uint i = 0; i < info.size(); i++) {
          printf("%-26s\t%s", (const char *)info[i]->pluginName().local8Bit(), (const char *)info[i]->comment().local8Bit());
          if(info[i]->isNoDisplay())
              std::cout << I18N_NOOP(" (Hidden)");
          std::cout << "\n";
      }
      std::cout << "\n" << I18N_NOOP("Note: Hidden plugins can not be disabled.") << "\n";
      return 0;
    }

    QCString p = args->getOption("p"), np = args->getOption("np");
    QStringList enabledPlugins, disabledPlugins;
    if( p.length() )
      enabledPlugins = QStringList::split(",", p);

    if( np.length() )
      disabledPlugins = QStringList::split(",", np);

    if( args->isSet("d") )
        scim::scim_daemon ();

    KApplication * kAppMainThread = new KApplication();
    if( kAppMainThread->isRestored() && !ScimKdeSettings::autoStart() )
      return 127; //when skim should not auto start, restore from session is not permitted

    if (signal(SIGTERM, sighandler) == SIG_IGN)
        signal(SIGTERM, SIG_IGN);
    if (signal(SIGINT, sighandler) == SIG_IGN)
        signal(SIGINT, SIG_IGN);
    if (signal(SIGHUP, sighandler) == SIG_IGN)
        signal(SIGHUP, SIG_IGN);

    new SkimPluginManager(enabledPlugins, disabledPlugins, otherArgs);
    return kAppMainThread->exec();
}
