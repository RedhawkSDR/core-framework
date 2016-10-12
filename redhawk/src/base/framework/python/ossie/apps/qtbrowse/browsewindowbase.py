# -*- coding: utf-8 -*-
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#


# Form implementation generated from reading ui file 'browsewindowbase.ui'
#
# Created: Thu Feb 4 17:22:26 2010
#      by: The PyQt User Interface Compiler (pyuic) 3.13
#
# WARNING! All changes made in this file will be lost!


from qt import *


class BrowseWindowBase(QMainWindow):
    def __init__(self,parent = None,name = None,fl = 0):
        QMainWindow.__init__(self,parent,name,fl)
        self.statusBar()

        if not name:
            self.setName("BrowseWindowBase")


        self.setCentralWidget(QWidget(self,"qt_central_widget"))
        BrowseWindowBaseLayout = QVBoxLayout(self.centralWidget(),11,6,"BrowseWindowBaseLayout")

        layout1 = QHBoxLayout(None,0,6,"layout1")

        self.textLabel1 = QLabel(self.centralWidget(),"textLabel1")
        layout1.addWidget(self.textLabel1)

        self.domainLabel = QLabel(self.centralWidget(),"domainLabel")
        layout1.addWidget(self.domainLabel)
        spacer1 = QSpacerItem(361,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout1.addItem(spacer1)
        BrowseWindowBaseLayout.addLayout(layout1)

        self.objectListView = QListView(self.centralWidget(),"objectListView")
        self.objectListView.addColumn(self.__tr("Object"))
        self.objectListView.header().setClickEnabled(0,self.objectListView.header().count() - 1)
        self.objectListView.addColumn(self.__tr("Description"))
        self.objectListView.header().setClickEnabled(0,self.objectListView.header().count() - 1)
        self.objectListView.setRootIsDecorated(1)
        BrowseWindowBaseLayout.addWidget(self.objectListView)

        layout12 = QGridLayout(None,1,1,0,6,"layout12")

        self.installButton = QPushButton(self.centralWidget(),"installButton")

        layout12.addWidget(self.installButton,0,1)

        self.createButton = QPushButton(self.centralWidget(),"createButton")
        self.createButton.setEnabled(0)

        layout12.addWidget(self.createButton,0,3)

        self.refreshButton = QPushButton(self.centralWidget(),"refreshButton")

        layout12.addWidget(self.refreshButton,0,0)

        self.uninstallButton = QPushButton(self.centralWidget(),"uninstallButton")
        self.uninstallButton.setEnabled(0)

        layout12.addWidget(self.uninstallButton,0,2)

        self.releaseButton = QPushButton(self.centralWidget(),"releaseButton")
        self.releaseButton.setEnabled(0)

        layout12.addWidget(self.releaseButton,0,4)

        self.stopButton = QPushButton(self.centralWidget(),"stopButton")
        self.stopButton.setEnabled(0)

        layout12.addWidget(self.stopButton,0,6)

        self.startButton = QPushButton(self.centralWidget(),"startButton")
        self.startButton.setEnabled(0)

        layout12.addWidget(self.startButton,0,5)
        BrowseWindowBaseLayout.addLayout(layout12)



        self.languageChange()

        self.resize(QSize(720,569).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.createButton,SIGNAL("clicked()"),self.createSelected)
        self.connect(self.installButton,SIGNAL("clicked()"),self.installApplication)
        self.connect(self.objectListView,SIGNAL("selectionChanged(QListViewItem*)"),self.selectionChanged)
        self.connect(self.refreshButton,SIGNAL("clicked()"),self.refreshView)
        self.connect(self.releaseButton,SIGNAL("clicked()"),self.releaseSelected)
        self.connect(self.startButton,SIGNAL("clicked()"),self.startSelected)
        self.connect(self.stopButton,SIGNAL("clicked()"),self.stopSelected)
        self.connect(self.uninstallButton,SIGNAL("clicked()"),self.uninstallSelected)
        self.connect(self.objectListView,SIGNAL("itemRenamed(QListViewItem*,int)"),self.propertyChanged)


    def languageChange(self):
        self.setCaption(self.__tr("REDHAWK Domin Browser"))
        self.textLabel1.setText(self.__tr("Domain:"))
        self.domainLabel.setText(QString.null)
        self.objectListView.header().setLabel(0,self.__tr("Object"))
        self.objectListView.header().setLabel(1,self.__tr("Description"))
        self.installButton.setText(self.__tr("&Install"))
        self.installButton.setAccel(self.__tr("Alt+I"))
        self.createButton.setText(self.__tr("&Create"))
        self.createButton.setAccel(self.__tr("Alt+C"))
        self.refreshButton.setText(self.__tr("&Refresh"))
        self.refreshButton.setAccel(self.__tr("Alt+R"))
        self.uninstallButton.setText(self.__tr("&Uninstall"))
        self.uninstallButton.setAccel(self.__tr("Alt+U"))
        self.releaseButton.setText(self.__tr("&Release"))
        self.releaseButton.setAccel(self.__tr("Alt+R"))
        self.stopButton.setText(self.__tr("S&top"))
        self.stopButton.setAccel(self.__tr("Alt+T"))
        self.startButton.setText(self.__tr("&Start"))
        self.startButton.setAccel(self.__tr("Alt+S"))


    def refreshView(self):
        print "BrowseWindowBase.refreshView(): Not implemented yet"

    def uninstallSelected(self):
        print "BrowseWindowBase.uninstallSelected(): Not implemented yet"

    def installApplication(self):
        print "BrowseWindowBase.installApplication(): Not implemented yet"

    def selectionChanged(self):
        print "BrowseWindowBase.selectionChanged(): Not implemented yet"

    def createSelected(self):
        print "BrowseWindowBase.createSelected(): Not implemented yet"

    def releaseSelected(self):
        print "BrowseWindowBase.releaseSelected(): Not implemented yet"

    def startSelected(self):
        print "BrowseWindowBase.startSelected(): Not implemented yet"

    def stopSelected(self):
        print "BrowseWindowBase.stopSelected(): Not implemented yet"

    def propertyChanged(self):
        print "BrowseWindowBase.propertyChanged(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("BrowseWindowBase",s,c)
