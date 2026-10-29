import os
import sys
from streamlit.web.cli import main

if __name__ == "__main__":
  # Descobre onde o executável/script está rodando de verdade (mesmo empacotado)
  if getattr(sys, "frozen", False):
    # Se estiver rodando como executável compilado pelo PyInstaller
    base_path = sys._MEIPASS
  else:
    # Se estiver rodando como script Python normal
    base_path = os.path.dirname(os.path.abspath(__file__))

  # Aponta o caminho exato do app.py embutido
  app_path = os.path.join(base_path, "app.py")

  # Configura os argumentos para o Streamlit rodar o app embutido
  sys.argv = ["streamlit", "run", app_path, "--server.headless=true"]
  sys.exit(main())