import QtQuick 2.0
import AsemanQml.Base 2.0
import Falcon.Base 1.0 as Falcon

Item {
    width: image.width
    height: image.height

    onWidthChanged: image.width = width
    onHeightChanged: image.height = height

    property alias source: file.source
    property alias socket: file.socket
    property alias progress: file.progress
    property alias downloading: file.downloading
    property alias details: file.details
    property alias receiveMethod: file.receiveMethod
    property alias destination: file.destination

    property alias fillMode: image.fillMode
    property alias asynchronous: image.asynchronous
    property alias cache: image.cache
    property alias horizontalAlignment: image.horizontalAlignment
    property alias mirror: image.mirror
    property alias paintedHeight: image.paintedHeight
    property alias paintedWidth: image.paintedWidth
    property alias smooth: image.smooth
    property alias sourceSize: image.sourceSize
    property alias status: image.status
    property alias verticalAlignment: image.verticalAlignment

    Image {
        id: image
        source: {
            var path = file.finalPath
            if(Tools.fileExists(path))
                return path
            else
                return ""
        }
    }

    Falcon.RemoteFile {
        id: file
        destination: {
            var temp = AsemanApp.tempPath
            Tools.mkDir(temp)
            return Devices.localFilesPrePath + temp + "/" + Tools.md5(source)
        }
        onError: console.debug(errorValue, errorCode)
    }
}
