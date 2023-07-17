import os

def get_latest_model_name():
    org_folder = "DeepCSR"
    
    # Cd into the org folder
    os.chdir(org_folder)

    # List all files
    model_n = os.listdir('.')[0]

    if model_n == 'objects':
        exit()
    else:
        return model_n

if __name__ == "__main__":

    print(get_latest_model_name())