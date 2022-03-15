import os
from PyQt5 import QtWidgets
from controller import Controller 
import threading

#os.system("pyuic5 " + os.getcwd() + "/MainWindow.ui -o " + os.getcwd() + "/MainWindow.py")

def main():
    print("### URABROS TESTER ###")
    app = QtWidgets.QApplication([])
    myController = Controller()
    app.exec_()

if __name__ == '__main__':
    main()