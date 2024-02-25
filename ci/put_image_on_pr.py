import os
import base64
from github import Github
from github import Auth
import sys

auth=Auth.Token(sys.argv[1])
g = Github(auth=auth)
repo = g.get_repo(os.getenv('GITHUB_REPOSITORY'))
pr_number = os.getenv('GITHUB_REF').split('/')[0]

# Read the image file
with open(sys.argv[2], 'rb') as image_file:
    base64_image = base64.b64encode(image_file.read()).decode('utf-8')
# Create the comment with the image
repo.get_pull(pr_number).create_issue_comment('![Image](data:image/png;base64,' + base64_image + ')')