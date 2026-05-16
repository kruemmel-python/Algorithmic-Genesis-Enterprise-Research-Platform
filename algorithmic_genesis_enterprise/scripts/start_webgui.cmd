@echo off
setlocal
cd /d "%~dp0\.."
python webui\server.py --host 127.0.0.1 --port 8765 --cli build_opencl\Release\ag_cli.exe
