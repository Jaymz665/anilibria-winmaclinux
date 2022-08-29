import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

ComboBox {
    id: dictionaryComboBox
    font.pointSize: 11 + windowSettings.dpiSeparation
    textRole: "title"
    delegate: ItemDelegate {
        width: dictionaryComboBox.width
        contentItem: Text {
            text: title
            color: dictionaryComboBox.currentIndex === index ? Material.accent : Material.foreground
            font.pointSize: 11 + windowSettings.dpiSeparation
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        highlighted: dictionaryComboBox.highlightedIndex === index
    }
}
