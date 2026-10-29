from datetime import datetime
import os
import time
import pandas as pd
import plotly.express as px
import serial
import streamlit as st

st.set_page_config(
    page_title="Monitor de Temperaturas", layout="wide"
)

st.title("🔥 Painel de Monitoramento de Temperatura e Corrente em Tempo Real")
st.write("Monitoramento de Sensores e Sistema Elétrico")

PORTA_SERIAL = "/dev/ttyUSB0"
BAUD_RATE = 9600

# Garante que a pasta 'log' existe e cria o arquivo exclusivo para esta sessão
os.makedirs("log", exist_ok=True)
tempo_inicio_programa = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
ARQUIVO_LOG = f"log/historico_{tempo_inicio_programa}.txt"


@st.cache_resource
def init_serial():
  try:
    ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
    time.sleep(2)
    return ser
  except Exception as e:
    st.error(f"Erro ao abrir a porta serial: {e}")
    return None


ser = init_serial()

# Criando colunas para os 6 sensores + 1 painel para a Corrente
st.markdown("### 📊 Valores Atuais")
cols = st.columns(7)  # 6 sensores + 1 corrente
metric_placeholders = []

# Criação das 6 caixas para os sensores de temperatura
for i in range(6):
  with cols[i]:
    container = st.container(border=True)
    with container:
      st.markdown(f"**Sensor {i+1}**")
      ph = st.empty()
      ph.metric(label=" ", value="-- °C")
      metric_placeholders.append(ph)

# 7ª Caixa dedicada para a Corrente Elétrica
with cols[6]:
  container_corrente = st.container(border=True)
  with container_corrente:
    st.markdown("**Corrente**")
    corrente_placeholder = st.empty()
    corrente_placeholder.metric(label=" ", value="-- A")

st.markdown("---")
st.subheader("📈 Histórico de Tendência em Tempo Real")
chart_placeholder = st.empty()

# DataFrame para armazenar o histórico com timestamps reais (6 sensores + Corrente)
colunas_df = ["Tempo"] + [f"Sensor {i+1}" for i in range(6)] + ["Corrente (A)"]
if "historico_dados" not in st.session_state:
  st.session_state.historico_dados = pd.DataFrame(columns=colunas_df)

if ser and ser.is_open:
  parar = st.button("Parar Monitoramento")

  while not parar:
    if ser.in_waiting > 0:
      linha = ser.readline().decode("utf-8").strip()
      valores_str = linha.split(",")

      # Esperamos 6 temperaturas + 1 corrente = 7 valores no total
      if len(valores_str) >= 7:
        try:
          # Converte os 6 primeiros valores para temperatura
          temperaturas = [
              float(v) if v != "NaN" else 0.0 for v in valores_str[:6]
          ]
          # O 7º valor é a corrente
          corrente_val = float(valores_str[6])

          timestamp_completo = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
          tempo_hora = datetime.now().strftime("%H:%M:%S")

          # Atualiza os valores visuais dos 6 sensores nas caixas
          for i in range(6):
            metric_placeholders[i].metric(
                label=" ", value=f"{temperaturas[i]:.2f} °C"
            )

          # Atualiza o valor visual da corrente na caixa dedicada
          corrente_placeholder.metric(label=" ", value=f"{corrente_val:.2f} A")

          # Salva os dados estruturados no arquivo .txt exclusivo dentro da pasta log/
          with open(ARQUIVO_LOG, "a", encoding="utf-8") as f:
            linha_txt = (
                f"{timestamp_completo}, "
                + ", ".join([f"{t:.2f}" for t in temperaturas])
                + f", {corrente_val:.2f}\n"
            )
            f.write(linha_txt)

          # Adiciona ao histórico global da sessão (para o gráfico)
          nova_linha = pd.DataFrame(
              [[tempo_hora] + temperaturas + [corrente_val]], columns=colunas_df
          )
          st.session_state.historico_dados = pd.concat(
              [st.session_state.historico_dados, nova_linha], ignore_index=True
          )

          # Mantém apenas as últimas 12 leituras no gráfico
          if len(st.session_state.historico_dados) > 12:
            st.session_state.historico_dados = (
                st.session_state.historico_dados.tail(12)
            )

          # Cria o gráfico profissional com Plotly focado nos 6 sensores
          fig = px.line(
              st.session_state.historico_dados,
              x="Tempo",
              y=[f"Sensor {i+1}" for i in range(6)],
              labels={
                  "value": "Temperatura (°C)",
                  "Tempo": "Horário da Leitura",
                  "variable": "Sensores",
              },
              template="plotly_dark",
          )
          
          # Configurações de layout e transição suave para evitar o "piscar"
          fig.update_layout(
              margin=dict(l=20, r=20, t=20, b=20),
              legend=dict(
                  orientation="h",
                  yanchor="bottom",
                  y=1.02,
                  xanchor="right",
                  x=1,
              ),
              transition={
                  'duration': 300, 
                  'easing': 'cubic-in-out'
              }
          )

          # Plota o gráfico suavizado no espaço reservado
          chart_placeholder.plotly_chart(fig, use_container_width=True, key=f"grafico_{time.time()}")

        except ValueError:
          pass

    time.sleep(0.5)
else:
  st.warning("⚠️ Verifique a conexão com a porta serial do Arduino.")