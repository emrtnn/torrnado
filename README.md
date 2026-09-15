# BitTorrent from First Principles

A learning-first project to build an interoperable, headless BitTorrent client
in modern C++ and understand the technology from its byte formats upward.

**The objective is learning, not shipping a client as quickly as possible.**
The learner writes the code. An AI assistant acts as a tutor and reviewer under
the instructions in [AGENTS.md](AGENTS.md), using explanations, questions,
experiments and progressive hints rather than implementing the project.

## Status

Planning only. This repository brief does not claim that source code, a build
system or any protocol feature already exists. Milestones below are targets,
not implemented capabilities. CLI examples are proposed interfaces.

## Why this project

Implementing a real protocol creates concrete reasons to understand binary
parsing, memory ownership, resource lifetimes, state machines, networking,
storage and performance. Interoperability provides an external correctness
check: our client must eventually exchange data with an established client.

By the end, the learner should be able to explain:

- What a `.torrent` file contains and what identifies a swarm.
- How peers find one another and agree which torrent they are discussing.
- How files, pieces and requested blocks relate to one another.
- How a client handles incomplete reads, peer state and outstanding requests.
- How data is verified, stored, resumed and uploaded.
- Why request pipelining and piece-selection policies affect throughput.
- How magnets, metadata exchange and decentralized peer discovery fit together.
- Where C++ ownership, allocation and concurrency decisions affect correctness
  and measured performance.

This is an independent educational project, not business infrastructure.

## Initial scope

Build a BitTorrent **v1** client, starting with one local peer and one file.
The first substantial release should handle one torrent at a time, IPv4/TCP,
`.torrent` input, HTTP tracker announces, compact peer lists, multiple peers,
verified downloads, basic uploads and resumption. Multi-file layout follows
single-file correctness.

Initially defer magnets, DHT, PEX, UDP trackers, uTP, protocol encryption,
automatic port mapping, v2, streaming, GUIs and multiple concurrent torrents.
These are optional later milestones, not prerequisites for meaningful progress.

Implement the BitTorrent logic ourselves. Libraries may provide socket I/O,
HTTP/TLS transport, hashing, test infrastructure and logging. Do not wrap
libtorrent as the client core; use it as a reference and interoperability peer.

Proposed tooling: C++23, CMake, Ninja, a supported GCC/Clang toolchain and a
Nix development environment if useful. Verify actual standard-library support.
Choose dependencies and build commands when the project is scaffolded; none
are assumed to exist yet.

## Learning workflow

For each milestone:

1. Read the relevant specification section, not every BEP.
2. Explain the problem and predict behavior on a small example.
3. Design the smallest useful implementation.
4. Write it and construct normal, boundary and malformed-input tests.
5. Observe actual behavior through tests, logs or packet captures.
6. Explain discrepancies and revise the implementation.
7. Compare against a reference implementation only when useful.

A milestone is complete when it works, relevant tests pass and the learner can
explain the core mechanism and at least one failure mode. There is no deadline
or requirement to implement every extension. Do not confuse implementing a
feature with needing the fastest possible implementation of it.

## Milestones

### 0. Establish the development loop

- [ ] A small executable and one automated test can be built and run.
- [ ] Understand compilation, linking and the selected build configuration.
- [ ] Enable useful warnings and a separate AddressSanitizer/UBSan build where
  supported; use ThreadSanitizer later when shared-memory concurrency appears.

Keep setup minimal. It should support learning, not become the first large task.

### 1. Bencode and a metadata inspector

- [ ] Decode byte strings, integers, lists and dictionaries incrementally in
  learning complexity; the initial file parser need not be streaming.
- [ ] Track consumed input and distinguish malformed/truncated input.
- [ ] Reject invalid encodings and enforce resource limits.
- [ ] Inspect a single-file `.torrent` and identify its `info` byte range.
- [ ] Compute its v1 info hash from the original encoded `info` bytes.
- [ ] Match the hash shown by an established client.

Read: [BEP 3: bencoding, metainfo and info-hash rules](https://www.bittorrent.org/beps/bep_0003.html).

Important distinction: bencoded strings are byte sequences, not necessarily
UTF-8 text. The info hash is not the hash of the entire `.torrent` file or of
the downloaded payload. Define and document a checked integer-representation
limit; do not silently overflow values that the format itself can express.

### 2. A controlled local peer and handshake

- [ ] Create a small test file and matching v1 torrent using an existing tool.
- [ ] Seed it with a reference client configured for local TCP testing.
- [ ] Accept an explicit peer endpoint, bypassing discovery for this exercise.
- [ ] Encode, send, receive and validate the handshake.
- [ ] Explain how partial reads/writes are handled.

Success: connect to the known peer, confirm the info hash and inspect its peer
ID and extension flags. Do not require public trackers or router changes.

Read: [BEP 3: peer protocol](https://www.bittorrent.org/beps/bep_0003.html).

### 3. Message framing and one verified piece

- [ ] Parse length-prefixed peer messages independently of socket I/O.
- [ ] Handle split messages, multiple messages in one read and keepalives.
- [ ] Track availability, interest and choking in both directions.
- [ ] Request blocks of an available piece and reconstruct it.
- [ ] Verify its SHA-1 against the metadata before marking it available.
- [ ] Reject invalid message sizes, indexes and offsets.

Success: save one verified piece and compare its bytes with the original.
TCP is a byte stream; a socket read is not a protocol-message boundary.

Read: [BEP 3: peer messages](https://www.bittorrent.org/beps/bep_0003.html).

### 4. A complete single-peer download

- [ ] Schedule and pipeline block requests with bounded memory use.
- [ ] Handle the final short piece/block, disconnects and request timeouts.
- [ ] Handle unexpected or duplicate responses and failed piece hashes.
- [ ] Write the complete single-file payload to a dedicated output directory.
- [ ] Compare the result byte-for-byte against the local original.

Success: repeatedly download the whole test file from the reference seeder.
Measure throughput only after correctness; predict why pipelining may help.

### 5. Tracker-based discovery

- [ ] Implement HTTP announce semantics and binary-safe query escaping.
- [ ] Parse failures, announce intervals and peer endpoints.
- [ ] Support compact IPv4 peer lists and regular announce events.
- [ ] Add tracker tiers/fallback as a separate increment.

Success: obtain peers without manually supplying an endpoint. Public tests
should use freely redistributable content and require an explicit run request.

Read: [BEP 3: trackers](https://www.bittorrent.org/beps/bep_0003.html),
[BEP 23: compact peers](https://www.bittorrent.org/beps/bep_0023.html),
[BEP 12: tracker tiers](https://www.bittorrent.org/beps/bep_0012.html).

### 6. Multiple peers and scheduling

- [ ] Introduce asynchronous or multiplexed I/O with clear buffer ownership.
- [ ] Separate per-peer state from torrent-wide progress.
- [ ] Track piece availability and implement a simple selection policy.
- [ ] Compare startup random selection with rarest-first selection.
- [ ] Reassign outstanding work when peers stall or disconnect.
- [ ] Add bounded endgame duplication/cancellation after basic scheduling works.

Success: complete a controlled download despite one peer disconnecting. Explain
which requests are outstanding, who owns their buffers and what can be retried.

Read: [libtorrent's piece-picker discussion](https://libtorrent.org/manual-ref.html)
and [Bram Cohen's incentives paper](https://www.bittorrent.org/bittorrentecon.pdf).

### 7. Uploading and seeding

- [ ] Accept incoming connections and advertise verified availability.
- [ ] Validate requests and serve only data we possess and have verified.
- [ ] Bound upload queues and implement basic choking/optimistic unchoking.
- [ ] Let an established client download and verify our complete test file.

Success: interoperability works in both directions, not just between two copies
of our own implementation. Separate required wire behavior from policy choices.

### 8. File layout, resumption and robustness

- [ ] Map v1 logical byte ranges across multiple files, including crossing pieces.
- [ ] Contain paths inside the output directory; consider traversal and symlinks.
- [ ] Recheck existing data and resume without trusting stale progress blindly.
- [ ] Handle disk errors, process interruption and bounded queues.
- [ ] Fuzz parsers and test corrupt peers and truncated data.

Success: interrupt and resume a multi-file download with identical final bytes,
without overwriting unrelated files or advertising invalid pieces.

### 9. Optional extensions, one at a time

| Extension | What it teaches | Specification |
| --- | --- | --- |
| UDP trackers | Datagram transactions, retries and correlation | [BEP 15](https://www.bittorrent.org/beps/bep_0015.html) |
| Extension negotiation | Per-peer capabilities and message identifiers | [BEP 10](https://www.bittorrent.org/beps/bep_0010.html) |
| Magnet metadata | Obtain and verify the info dictionary from peers | [BEP 9](https://www.bittorrent.org/beps/bep_0009.html), after BEP 10 |
| DHT | Kademlia-style lookup, routing buckets and UDP RPC | [BEP 5](https://www.bittorrent.org/beps/bep_0005.html) |
| Peer Exchange | Discover additional peers through existing connections | [BEP 11](https://www.bittorrent.org/beps/bep_0011.html), after BEP 10 |
| Fast extension | Additional request/choke semantics | [BEP 6](https://www.bittorrent.org/beps/bep_0006.html) |
| uTP | Reliable transport and congestion control over UDP | [BEP 29](https://www.bittorrent.org/beps/bep_0029.html) |
| BitTorrent v2 | SHA-256 Merkle trees and changed metadata/layout rules | [BEP 52](https://www.bittorrent.org/beps/bep_0052.html) |

Magnets do not require DHT if peers can be found another way, such as a tracker
included in the URI. DHT returns peer contacts, not the metadata or payload.
PEX needs an existing connection; it cannot bootstrap from nothing. Respect
[private-torrent restrictions](https://www.bittorrent.org/beps/bep_0027.html)
before enabling additional discovery methods.

## Design questions to discover, not pre-answer

- Should the parser own its data or borrow views into an input buffer?
- How do incomplete input and invalid input differ at each boundary?
- What owns a connection, an outstanding request and a piece buffer?
- Which state belongs to a peer and which belongs to the torrent?
- How do we prevent a slow disk from causing unbounded network buffers?
- Which errors are recoverable, and at what layer?
- What can a successful hash check establish, and what can it not establish?
- What should happen if a peer disconnects after sending only part of a block?

There is deliberately no mandatory class hierarchy or complete solution here.
Keep modules small and boundaries testable; let the design emerge from actual
requirements. Do not force every advanced C++ feature into the project.

## Working with the AI tutor

Useful prompts:

- "Explain the next concept, then give me one small exercise without a solution."
- "Review my reasoning before reviewing my code."
- "Give me the smallest hint that would unblock this test."
- "Help me trace these received bytes; don't fix my parser."
- "What experiment would distinguish these two explanations?"
- "I explicitly want a worked example of this one C++ mechanism."

The tutor should answer conceptual questions clearly, not withhold explanations
in the name of discovery. Full implementations are an explicit opt-in exception.

## References

- [Official BEP index](https://www.bittorrent.org/beps/bep_0000.html): navigation,
  not a curriculum to read in numeric order.
- [BEP 3](https://www.bittorrent.org/beps/bep_0003.html): main v1 starting point;
  some historical wording must be read alongside later extensions.
- [Incentives Build Robustness in BitTorrent](https://www.bittorrent.org/bittorrentecon.pdf):
  design motivation for piece selection and peer cooperation.
- [libtorrent source](https://github.com/arvidn/libtorrent) and
  [Transmission source](https://github.com/transmission/transmission): reference
  implementations and interoperability peers, not code to copy wholesale.
- [Wireshark BitTorrent page and captures](https://wiki.wireshark.org/BitTorrent):
  inspect controlled traffic; use BEPs, not old wiki prose, as protocol authority.
- [Boost.Asio tutorial](https://www.boost.org/doc/libs/latest/doc/html/boost_asio/tutorial.html):
  an optional route from synchronous sockets to asynchronous networking.

## The first exercise

Read only the bencoding section of BEP 3. Work out how `4:spam` represents a byte
string and what should happen for `4:spa`. Then implement the smallest decoder
for that one type, with tests and a clear consumed-input/error contract.

Do not begin by writing the whole client. Begin by understanding the first bytes.
