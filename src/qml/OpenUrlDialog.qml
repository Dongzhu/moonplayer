/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */
 
import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import CustomWidgets 1.0

Dialog {
    id: openUrlDialog
    width: 350
    height: 210
    title: qsTr("Enter URL to parse")
    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: {
        if (openUrlInput.text !== "")
            playlistModel.addUrl(openUrlInput.text, downloadCheckBox.checked);
        openUrlInput.text = "";
    }

    onRejected: openUrlInput.text = ""
    
    Connections {
        target: playlistModel
        onUrlDialogRequested: {
            openUrlInput.text = url
            openUrlDialog.open()
        }
    }

    ColumnLayout {
        anchors.fill: parent

        TextField {
            id: openUrlInput
            selectByMouse: true
            Layout.fillWidth: true
            height: 30
            onAccepted: openUrlDialog.accept()
        }

        CheckBox {
            id: downloadCheckBox
            text: qsTr("Download video")
        }
    }
}
