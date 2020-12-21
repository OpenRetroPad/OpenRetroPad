Import("env")

board_config = env.BoardConfig()
# should be array of VID:PID pairs
board_config.update("build.hwids", [
  ["0x0f0d", "0x00c1"], # Hori Co., Ltd HORIPAD for Nintendo Switch
])
