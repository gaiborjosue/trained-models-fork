import os

import json

def get_push_commit_message():
    event_path = os.environ.get('GITHUB_EVENT_PATH')

    if not event_path:
        print("Error: GITHUB_EVENT_PATH not set.")
        return None

    with open(event_path, 'r') as event_file:
        event_data = json.load(event_file)

    if 'commits' in event_data and len(event_data['commits']) > 0:
        commit_message = event_data['commits'][0]['message']
        org_folder = commit_message.split(':', 1)[0].strip()
        return org_folder
    else:
        print("Error: No commit messages found in the event data.")
        return None
def get_latest_model_name():
    org_folder = get_push_commit_message()
    
    # Cd into the org folder
    os.chdir(org_folder)

    # Assign current directory to model_n
    model_n = os.getcwd()

    if model_n == 'objects':
        exit()
    else:
        return model_n

def get_dockerfile_path(model_folder):
    for root, _, files in os.walk(model_folder):
        for file in files:
            if file.lower() == "dockerfile":
                return os.path.join(root)
    return None

if __name__ == "__main__":
    model_folder = get_latest_model_name()

    dockerfile_path = get_dockerfile_path(model_folder)
    if dockerfile_path:
        print(dockerfile_path)
    else:
        print("Dockerfile not found.")