/* -*- c++ -*- */
/*
 * Copyright 2011-2012 Alexandru Csete OZ9AEC.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QString>

#include "mainwindow.h"
#include "gqrx.h"

#include <iostream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

static void reset_conf(const QString &file_name);

int main(int argc, char *argv[])
{
    std::string conf;
    bool clierr=false;

    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(GQRX_ORG_NAME);
    QCoreApplication::setOrganizationDomain(GQRX_ORG_DOMAIN);
    QCoreApplication::setApplicationName(GQRX_APP_NAME);
    QCoreApplication::setApplicationVersion(VERSION);

    //setup the program options
    po::options_description desc("Command line options");
    desc.add_options()
        ("help,h", "This help message")
        ("conf,c", po::value<std::string>(&conf), "Start with this config file")
        ("edit,e", "Edit the config file before using it (not implemented)")
        ("reset,r", "Reset configuration file")
    ;

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch(const boost::program_options::invalid_command_line_syntax& ex)
    {
        /* happens if e.g. -c without file name */
        clierr = true;
    }

    po::notify(vm);

    // print the help message
    if (vm.count("help") || clierr)
    {
        std::cout << "Gqrx software defined radio receiver " << VERSION << std::endl << desc << std::endl;
        return 1;
    }

    if (!conf.empty())
    {
        qDebug() << "User specified config file:" << QString::fromStdString(conf);
        if (vm.count("reset"))
            reset_conf(QString::fromStdString(conf));
    }
    else
    {
        qDebug() << "No user supplied config file. Using default.";
        if (vm.count("reset"))
            reset_conf("default.conf");
    }

    // Mainwindow will check whether we have a configuration
    // and open the config dialog if there is none or the specified
    // file does not exist.
    MainWindow w(conf.empty() ? "default.conf" : QString::fromStdString(conf));

    if (w.configOk)
    {
        w.show();
        return a.exec();
    }
    else
    {
        return 1;
    }
}

/*! \brief Reset configuration file specified by file_name. */
static void reset_conf(const QString &file_name)
{
    QString cfg_file;
    QByteArray xdg_dir = qgetenv("XDG_CONFIG_HOME");

    if (xdg_dir.isEmpty())
        cfg_file = QString("%1/.config/gqrx/%2").arg(QDir::homePath()).arg(file_name);
    else
        cfg_file = QString("%1/gqrx/%2").arg(xdg_dir.data()).arg(file_name);

    if (QFile::exists(cfg_file))
    {
        if (QFile::remove(cfg_file))
            qDebug() << cfg_file << "deleted";
        else
            qDebug() << "Failed to remove" << cfg_file;
    }
    else
    {
        qDebug() << "Cano not delete" << cfg_file << "- file does not exist!";
    }
}
