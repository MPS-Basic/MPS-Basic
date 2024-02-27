import pyimgur
import sys

CLIENT_ID = sys.argv[2]

im = pyimgur.Imgur(CLIENT_ID)
uploaded_image = im.upload_image(sys.argv[1], title="tmp")
print(uploaded_image.link)
