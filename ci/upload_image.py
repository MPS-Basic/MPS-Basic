import pyimgur
import sys

CLIENT_ID = "a0a98f5baed7b8f"

im = pyimgur.Imgur(CLIENT_ID)
print(sys.argv[0])
uploaded_image = im.upload_image(sys.argv[0], title="tmp")
print(uploaded_image.link)
