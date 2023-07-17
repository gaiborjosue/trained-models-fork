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

    # List all files
    model_n = os.listdir('.')[0]

    if model_n == 'objects':
        exit()
    else:
        return model_n

if __name__ == "__main__":

    print(get_latest_model_name())