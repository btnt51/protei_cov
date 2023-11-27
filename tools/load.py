import concurrent.futures
import http.client

def send_request(host, port, path):
    conn = http.client.HTTPConnection(host, port)
    conn.request("GET", path)
    response = conn.getresponse()
    data = response.read()
    print(f"Response from {host}:{port}{path}:")
    print(f"Response Code: {response.status}")
    print("Response Data:")
    print(data.decode())
    conn.close()

def main():
    nums = [str(x) for x in range(140)]

    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = [executor.submit(send_request, "localhost", 8080, f"/phone={num}") for num in nums]

    # Wait for all futures to complete
    concurrent.futures.wait(futures)

if __name__ == "__main__":
    main()