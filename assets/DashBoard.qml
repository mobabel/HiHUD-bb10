/* Copyright (c) 2012, 2013  BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import bb.cascades 1.3

// A page that lists all services of a remote bluetooth device
Page {
    //! [0]
    actions: [
        ActionItem {
            title: qsTr("Device Info")
            imageSource: "asset:///images/local_device.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                qsDeviceInfo.open();
            }
        },
        ActionItem {
            title: qsTr("Service List")
            imageSource: "asset:///images/broadcasts.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                //_btController.readData();
                //_btController.chatManager.connectToSPPService();
                //navigationPane.push(chatPage.createObject());
                navigationPane.push(remoteDevicePageDefinition.createObject())
            }
        }
    ]
    //! [0]

    Container {
        Container {
            //! [1]
            id: dashBoardContainer
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Center
            layout: StackLayout {
            }
            leftPadding: 10.0
            rightPadding: 10.0
            
            Container {
                background: Color.Black
                preferredHeight: ui.du(60)
                topPadding: ui.du(2.2)
                //! [1]
                TextArea {
                    textStyle {
                        base: SystemDefaults.TextStyles.SubtitleText
                        color: Color.White
                    }
                    backgroundVisible: true
                    editable: false
                    text: _btController.logHistory
                    maximumLength: 1000000
                }
                //! [1]
            }
            Container {
                topMargin: 5.0
                layout: DockLayout {
                }
                preferredWidth: maxWidth
                Label {
                    text: _dashBoard.rpm
                    horizontalAlignment: HorizontalAlignment.Left
                    verticalAlignment: VerticalAlignment.Center
                    textStyle.fontSize: FontSize.PointValue
                    textStyle.fontWeight: FontWeight.Bold
                    textStyle.base: digitalStyle.style
                    textStyle.fontSizeValue: 0.0
                    textFit.minFontSizeValue: 20.0
                    textFit.maxFontSizeValue: 20.0
                    textStyle.color: Color.Green
                }
                Label {
                    text: _dashBoard.speed
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    textStyle.fontSize: FontSize.PointValue
                    textStyle.fontWeight: FontWeight.Bold
                    textStyle.base: digitalStyle.style
                    textStyle.fontSizeValue: 0.0
                    textFit.minFontSizeValue: 50.0
                    textFit.maxFontSizeValue: 50.0
                    textStyle.color: Color.Green
                }
            }
            
            Container {
                topMargin: 5.0
                layout: DockLayout {
                }
                preferredWidth: maxWidth
                visible: false
                Label {
                    text: qsTr("km/h")
                    horizontalAlignment: HorizontalAlignment.Left
                    verticalAlignment: VerticalAlignment.Bottom
                    textStyle.fontSize: FontSize.PointValue
                    textStyle.fontWeight: FontWeight.Bold
                    textStyle.base: digitalStyle.style
                    textStyle.fontSizeValue: 0.0
                    textFit.minFontSizeValue: 15.0
                    textFit.maxFontSizeValue: 15.0
                    textStyle.color: Color.Green
                
                }
                Label {
                    text: qsTr("100")
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    textStyle.fontSize: FontSize.PointValue
                    textStyle.fontWeight: FontWeight.Normal
                    textStyle.base: digitalStyle.style
                    textStyle.fontSizeValue: 0.0
                    textFit.minFontSizeValue: 40.0
                    textFit.maxFontSizeValue: 40.0
                    textStyle.color: Color.Cyan
                    textStyle.textAlign: TextAlign.Right
                    scaleX: 1.0

                }
            }
            //! [1]
        }

        //! [2]
        attachedObjects: [
            RemoteDeviceInfoSheet {
                id: qsDeviceInfo
            },
            ComponentDefinition {
                id: remoteDevicePageDefinition
                source: "RemoteDevice.qml"
            },
            TextStyleDefinition {
                id: digitalStyle
                base: SystemDefaults.TextStyles.BodyText
                rules: [
                    FontFaceRule {
                        source: "fonts/digifacewideregular.ttf"
                        fontFamily: "DigitalFont"
                    }
                ]
                fontFamily: "DigitalFont, sans-serif"
            },
            TextStyleDefinition {
                id: digitalEmaStyle
                base: SystemDefaults.TextStyles.BodyText
                rules: [
                    FontFaceRule {
                        source: "fonts/digitalema.ttf"
                        fontFamily: "DigitalEmaFont"
                    }
                ]
                fontFamily: "DigitalEmaFont, sans-serif"
            }
        ]
        //! [2]
    }
}
