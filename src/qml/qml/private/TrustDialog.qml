import AsemanQml.Base 2.0
import AsemanQml.Awesome 2.0
import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Controls 2.1 as QtControls
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.1

Dialog {
    id: trustDialog
    title: qsTr("Trusting")
    x: parent.width/2 - width/2
    y: parent.height/2 - height/2
    dim: true
    modal: true
    closePolicy: Popup.CloseOnPressOutside
    standardButtons: QtControls.Dialog.Ok | QtControls.Dialog.Cancel

    onVisibleChanged: {
        if(visible)
            BackHandler.pushHandler(this, function(){visible = false})
        else
            BackHandler.removeHandler(this)
    }

    property var lastCallback
    property variant coreServices

    function checkTrust(callback) {
        coreServices.auth.isTrusted( function(res, error){
            if(!res) {
                trustDialog.open()
                lastCallback = callback
            } else if(callback)
                callback()
        })
    }

    ColumnLayout {
        Label {
            text: qsTr("Is it you?")
        }

        TextField {
            id: passwordField
            inputMethodHints: Qt.ImhNoPredictiveText
            selectByMouse: true
            placeholderText: qsTr("Password")
            echoMode: TextInput.Password
            passwordMaskDelay: 1000
            passwordCharacter: '*'
            color: {
                if(focus || errorAvailable == -1)
                    return Material.foreground
                if(errorAvailable)
                    return "#ff0000"
                else
                    return Material.color(Material.Teal)
            }
            Layout.preferredWidth: {
                var res = trustDialog.parent.width - 100*Devices.density
                if(res > 350*Devices.density)
                    res = 350*Devices.density
                return res
            }
            onAccepted: trustDialog.accept()

            onTextChanged: errorAvailable = (text.length < 8? 1 : 0)

            property int errorAvailable: -1
        }
    }

    onAccepted: {
        coreServices.auth.trustPassword(passwordField.text, function(res, error){
            if(!res) {
                showTooltip( qsTr("Feiled") )
                open()
            }
            else if(lastCallback)
                lastCallback()
        })
    }
}
