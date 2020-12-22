Import("env")

board_config = env.BoardConfig()
# should be array of VID:PID pairs
board_config.update("build.hwids", [
  ["0x2dc8", "0x6002"], # 8BitDo 8BitDo SN30 Pro+
])
