#View
from PyQt5 import QtCore
from PyQt5.QtCore import QObject, pyqtSignal
from PyQt5 import QtWidgets
from MainWindow import Ui_Form

class View(QtWidgets.QWidget):
  def __init__(self):
    super(View, self).__init__()
    self.ui = Ui_Form()
    self.ui.setupUi(self)