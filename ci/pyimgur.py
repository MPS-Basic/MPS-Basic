import pyimgur
import sys

CLIENT_ID = "a0a98f5baed7b8f"
PATH = sys.argv[0] + "/output_001.png"

uploaded_image = im.uploaded_image(PATH, title="tmp")
print(uploaded_image.link)
