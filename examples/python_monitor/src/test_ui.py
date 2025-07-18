import tkinter as tk
from tkinter import messagebox

# Se i bindings sono disponibili, importarli (placeholder)
try:
    from python_bindings import ErrorHandler
    handler = ErrorHandler()
    handler.initialize()
except ImportError:
    handler = None


def log_info():
    if handler:
        handler.info("Messaggio di info dalla GUI Python!")
    messagebox.showinfo("Info", "Messaggio di info inviato al core!")

def log_error():
    if handler:
        handler.error("Errore simulato dalla GUI Python!")
    messagebox.showerror("Errore", "Errore simulato inviato al core!")

def quit_app():
    if handler:
        handler.shutdown()
    root.destroy()

root = tk.Tk()
root.title("Test GUI Python - CBA")

frame = tk.Frame(root, padx=20, pady=20)
frame.pack()

btn_info = tk.Button(frame, text="Invia Info", command=log_info)
btn_info.pack(pady=5)

btn_error = tk.Button(frame, text="Invia Errore", command=log_error)
btn_error.pack(pady=5)

btn_quit = tk.Button(frame, text="Esci", command=quit_app)
btn_quit.pack(pady=10)

root.mainloop() 