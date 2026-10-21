import time
import pandas as pd
import plotly.express as px
import serial
import streamlit as st

st.set_page_config(
    page_title="Monitor de Temperaturas", layout="wide"
)

st.title("🔥 Painel de Monitoramento de Temperatura em Tempo Real")
st.write(
    "Monitoramento de Sensores"
)

PORTA_SERIAL = "/dev/ttyUSB0"
BAUD_RATE = 9600


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

# Criando colunas com caixas/bordas visuais para cada sensor
st.markdown("### Valores Atuais")
cols = st.columns(5)
metric_containers = []
metric_placeholders = []

for i in range(5):
  with cols[i]:
    # Cria uma caixinha com borda para cada sensor
    container = st.container(border=True)
    with container:
      st.markdown(f"**Sensor {i+1}**")
      ph = st.empty()
      ph.metric(label="", value="-- °C")
      metric_placeholders.append(ph)

st.markdown("---")
st.subheader("📊 Histórico de Tendência em Tempo Real")
chart_placeholder = st.empty()

# DataFrame para armazenar o histórico com timestamps reais
colunas_df = ["Tempo"] + [f"Sensor {i+1}" for i in range(5)]
if "historico_dados" not in st.session_state:
  st.session_state.historico_dados = pd.DataFrame(columns=colunas_df)

if ser and ser.is_open:
  parar = st.button("Parar Monitoramento")

  while not parar:
    if ser.in_waiting > 0:
      linha = ser.readline().decode("utf-8").strip()
      valores_str = linha.split(",")

      if len(valores_str) >= 5:
        try:
          temperaturas = [
              float(v) if v != "NaN" else 0.0 for v in valores_str[:5]
          ]
          tempo_atual = pd.Timestamp.now().strftime("%H:%M:%S")

          # Atualiza os valores dentro das caixas
          for i in range(5):
            metric_placeholders[i].metric(
                label="", value=f"{temperaturas[i]:.2f} °C"
            )

          # Adiciona ao histórico global da sessão
          nova_linha = pd.DataFrame(
              [[tempo_atual] + temperaturas], columns=colunas_df
          )
          st.session_state.historico_dados = pd.concat(
              [st.session_state.historico_dados, nova_linha], ignore_index=True
          )

          # Mantém apenas as últimas 40 leituras para não sobrecarregar o gráfico
          if len(st.session_state.historico_dados) > 40:
            st.session_state.historico_dados = (
                st.session_state.historico_dados.tail(40)
            )

          # Cria o gráfico profissional com Plotly
          fig = px.line(
              st.session_state.historico_dados,
              x="Tempo",
              y=[f"Sensor {i+1}" for i in range(5)],
              labels={
                  "value": "Temperatura (°C)",
                  "Tempo": "Horário da Leitura",
                  "variable": "Sensores",
              },
              template="plotly_dark",
          )
          fig.update_layout(
              margin=dict(l=20, r=20, t=20, b=20),
              legend=dict(
                  orientation="h",
                  yanchor="bottom",
                  y=1.02,
                  xanchor="right",
                  x=1,
              ),
          )

          chart_placeholder.plotly_chart(fig, use_container_width=True)

        except ValueError:
          pass

    time.sleep(0.5)
else:
  st.warning("⚠️ Verifique a conexão com a porta serial do Arduino.")