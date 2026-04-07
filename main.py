from google import genai

client = genai.Client(api_key="AIzaSyDKJyg51a3on7pSrA0RHOdcde9WKmQKO60")

print(f"{'Model Name':<30} | {'Supported Methods'}")
print("-" * 60)

for model in client.models.list():
    # Filter for Gemma models specifically
    if "gemma" in model.name.lower():
        methods = ", ".join(model.supported_actions)
        print(f"{model.name:<30} | {methods}")
