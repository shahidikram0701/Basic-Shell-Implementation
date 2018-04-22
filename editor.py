import sys
import os
from tkinter import *
import tkinter.filedialog as fdialog

root=Tk()
text=Text(root)
text.grid()

def open_cmd():
    file = fdialog.askopenfile(parent=root,mode='rb',title='Select a file')
    if file != None:
        contents = file.read()
        text.insert('1.0',contents)
        file.close()

def saveas():
    global text
    t = text.get("1.0", "end-1c")
    savelocation=fdialog.asksaveasfilename()
    file1=open(savelocation, "w+")
    file1.write(t)
    file1.close()
    os.chmod(savelocation, 0o777)
    
button=Button(root, text="Save", command=saveas)
button.grid()
button2 = Button(root, text="Open", command=open_cmd)
button2.grid()
root.mainloop()
