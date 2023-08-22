import subprocess

def run_command(command:str):
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    output, error = process.communicate()

    output = output.decode('utf-8')
    error = error.decode('utf-8')
    return output, error
