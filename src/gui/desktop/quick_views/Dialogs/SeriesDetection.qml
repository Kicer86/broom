
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "../Components" as Components
import "DialogsComponents" as Internals

/*
 * Series detection dialog
 */

Item
{
    id: seriesDetectionMainId
    objectName: "seriesDetectionMain"

    state: "LoadingState"

    signal group(int index)

    RowLayout {
        id: groupsId
        anchors.fill: parent

        ColumnLayout {
            id: column
            width: 200
            height: 400

            Text {
                id: element
                text: qsTr("Group candidates")
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                font.pixelSize: 12
            }

            ListView {
                id: groupsListId
                clip: true
                Layout.fillWidth: true
                Layout.fillHeight: true

                property alias thumbnailSize: thumbnailSliderId.size

                model: groupsModelId

                highlightMoveDuration : 200
                highlightMoveVelocity : -1

                ScrollBar.vertical: ScrollBar { }

                delegate: Item {
                    id: delegateId

                    width: delegateId.ListView.view.width       // using 'parent' causes erros in output after thumbnail being resized
                    height: groupsListId.thumbnailSize

                    // from view
                    required property int index

                    // from model - roles
                    required property var photoData
                    required property var groupType
                    required property var members

                    Row {
                        anchors.fill: parent

                        Internals.PhotoDelegate {
                            id: photoDelegateId

                            width: parent.height
                            height: parent.height
                        }

                        Item {
                            width: parent.width - photoDelegateId.width
                            height: parent.height

                            Text {
                                text: groupType

                                anchors.bottom: membersList.top
                            }

                            ListView {
                                id: membersList

                                width: parent.width
                                height: groupsListId.thumbnailSize - 50
                                anchors.bottom: parent.bottom

                                visible: groupsListId.thumbnailSize > 100

                                orientation: ListView.Horizontal
                                model: members

                                delegate: Internals.PhotoDelegate
                                {
                                    width: membersList.height
                                    height: membersList.height
                                }
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: delegateId.ListView.view.currentIndex = index
                    }
                }

                highlight: Rectangle {
                    id: highlightId
                    color: "lightsteelblue"
                    opacity: 0.4
                    z: -1
                }

                Components.ThumbnailSlider {
                    id: thumbnailSliderId
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                }
            }

            Button {
                id: button
                text: qsTr("Group", "used as verb - group photos")
                enabled: groupsListId.currentIndex != -1

                Connections {
                    target: button
                    function onClicked() {
                        seriesDetectionMainId.group(groupsListId.currentIndex)
                    }
                }
            }
        }
    }

    Item {
        id: loadingId
        anchors.fill: parent

        Item {
            id: containerId
            width: childrenRect.width
            height: childrenRect.height
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            // TODO: For some reason BusyIndicator causes
            // 'QQmlEngine::setContextForObject(): Object already has a QQmlContext'
            // error in output
            BusyIndicator {
                id: busyIndicatorId
                anchors.top: infoId.bottom
                anchors.topMargin: 0
                anchors.horizontalCenter: infoId.horizontalCenter
            }

            Text {
                id: infoId
                text: qsTr("Looking for group candidates...")
                anchors.top: parent.top
                font.pixelSize: 12
            }
        }
    }

    states: [
        State {
            name: "LoadingState"

            PropertyChanges {
                target: groupsId
                opacity: 0
            }
        },
        State {
            name: "LoadedState"
            when: groupsModelId.loaded
        }
    ]

    transitions:
        Transition {
        from: "LoadingState"
        to: "LoadedState"
        ParallelAnimation {
            PropertyAnimation {
                target: loadingId
                properties: "opacity"
                from: 1
                to: 0
            }
            PropertyAnimation {
                target: groupsId
                properties: "opacity"
                from: 0
                to: 1
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
