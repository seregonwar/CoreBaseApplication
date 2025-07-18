import sys
import os

# Aggiungi il percorso del modulo compilato al PYTHONPATH
module_path = os.path.join(os.path.dirname(__file__), '..', '..', '..', 'build', 'lib', 'Release')
sys.path.insert(0, os.path.abspath(module_path))

# Importa il modulo python_bindings compilato
import python_bindings

print("=== Test Python Bindings per CoreBaseApplication ===")
print(f"Modulo caricato: {python_bindings.__name__}")
print(f"Funzioni disponibili: {[attr for attr in dir(python_bindings) if not attr.startswith('_')]}")

# Test della funzione add
print("\n=== Test funzione add ===")
result = python_bindings.add(5, 3)
print(f"add(5, 3) = {result}")

# Test della classe ErrorHandler
print("\n=== Test ErrorHandler ===")
error_handler = python_bindings.ErrorHandler()
print("ErrorHandler creato con successo")

# Test dei metodi di logging
error_handler.info("Messaggio di info dal test Python")
error_handler.warning("Messaggio di warning dal test Python")
error_handler.error("Messaggio di errore dal test Python")
error_handler.debug("Messaggio di debug dal test Python")
error_handler.critical("Messaggio critico dal test Python")
print("Messaggi di log inviati con successo")

# Test della classe PythonScriptingEngine
print("\n=== Test PythonScriptingEngine ===")
scripting_engine = python_bindings.PythonScriptingEngine()
print("PythonScriptingEngine creato con successo")

# Inizializza il motore di scripting
if scripting_engine.initialize():
    print("PythonScriptingEngine inizializzato con successo")
    
    # Test esecuzione di codice Python
    test_code = "result = 2 + 3; print(f'Risultato: {result}')"
    if scripting_engine.execString(test_code):
        print("Codice Python eseguito con successo")
    else:
        print("Errore nell'esecuzione del codice Python")
    
    # Finalizza il motore
    scripting_engine.finalize()
    print("PythonScriptingEngine finalizzato")
else:
    print("Errore nell'inizializzazione di PythonScriptingEngine")

# Test della funzione execute_python_script_with_output
print("\n=== Test execute_python_script_with_output ===")
script_output = python_bindings.execute_python_script_with_output("print('Hello from executed script!')")
print(f"Output dello script: {script_output}")

print("\n=== Test completato con successo! ===")