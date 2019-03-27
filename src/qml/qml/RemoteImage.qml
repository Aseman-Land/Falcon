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
    property alias destination: file.destPathBase

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
            if(refresher < 0)
                return ""

            var path = file.destPathBase
            if(Tools.fileExists(path))
                return path
            else
                return ""
        }
        property int refresher: 0
    }

    Falcon.RemoteFile {
        id: file
        destination: {
            if(Tools.fileExists(destPathBase))
                return destPathBase
            else
                return destPathDownloading
        }
        onError: console.debug(errorValue, errorCode)
        onDownloadingChanged: {
            if(downloading)
                return

            Tools.rename( Tools.urlToLocalPath(destPathDownloading), Tools.urlToLocalPath(destPathBase) )
            image.refresher++
        }

        property string destPathBase: {
            var temp = AsemanApp.tempPath

            var suffix = ""
            var suffixIdx = (source + "").lastIndexOf(".")
            if(suffixIdx >= 0)
                suffix = source.slice(suffixIdx)

            Tools.mkDir(temp)
            return Devices.localFilesPrePath + temp + "/" + Tools.md5(source) + suffix
        }
        property string destPathDownloading: destPathBase + ".download"
    }
}
