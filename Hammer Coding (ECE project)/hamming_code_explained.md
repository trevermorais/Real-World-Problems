# 💡 Hamming Code, Explained Simply

> **One sentence:** Hamming code lets a computer **spot and fix a mistake in a message all by itself**, without asking for the message to be sent again.

---

## 1. The Story (read this first)

Imagine you send **4 light bulbs** to a friend through a long, dark tunnel.

- Each bulb is **ON (1)** or **OFF (0)**. That pattern is your secret message.
- A naughty **gremlin 😈** lives in the tunnel. Sometimes he flips **one** bulb: ON becomes OFF, or OFF becomes ON.
- Your friend can't call you and ask "what did you mean?"

**The trick:** before sending, you add **3 helper bulbs**. Three buddies (Ruby 🔴, Bluey 🔵 and Leafy 🟢) each look after a few seats. If the gremlin strikes, the buddies can tell **which seat** was changed, and your friend flips it back. 🎉

> Real life: phones, computer memory (ECC RAM), satellites and QR codes all use this idea, because tiny errors happen everywhere.

---

## 2. The Only Rule: Pairs

Each buddy has **one rule**:

> "The lit bulbs I watch must come in **pairs**."

That means an **even** number of lit bulbs (0, 2 or 4).

| Lit bulbs | Pairs? | Verdict |
|:---------:|:------:|---------|
| 0 | ✅ yes | Even, happy |
| 1 | ❌ one left out | **Odd, problem!** |
| 2 | ✅ yes | Even, happy |
| 3 | ❌ one left out | **Odd, problem!** |
| 4 | ✅ yes | Even, happy |

This is called **even parity** in computer language. You can forget that word. Just think "pairs".

---

## 3. The Seating Plan

There are **7 seats**. Seats **1, 2 and 4** belong to the helpers. Your 4 message bulbs sit in seats **3, 5, 6 and 7**.

| Seat | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|:----:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| Who sits here | 🔴 Ruby | 🔵 Bluey | You (1st bit) | 🟢 Leafy | You (2nd bit) | You (3rd bit) | You (4th bit) |

Each buddy watches **4 seats** (their own plus three others):

| Buddy | Watches seats |
|-------|---------------|
| 🔴 Ruby (seat 1) | 1, 3, 5, 7 |
| 🔵 Bluey (seat 2) | 2, 3, 6, 7 |
| 🟢 Leafy (seat 4) | 4, 5, 6, 7 |

**Why those seats?** Write each seat number in binary (the 0s and 1s language). A buddy watches every seat that has a `1` in *their* column:

| Seat | Binary | Ruby (rightmost) | Bluey (middle) | Leafy (leftmost) |
|:----:|:------:|:---:|:---:|:---:|
| 1 | 001 | ✔ | | |
| 2 | 010 | | ✔ | |
| 3 | 011 | ✔ | ✔ | |
| 4 | 100 | | | ✔ |
| 5 | 101 | ✔ | | ✔ |
| 6 | 110 | | ✔ | ✔ |
| 7 | 111 | ✔ | ✔ | ✔ |

👉 **Every seat has a different combination of buddies.** That's the whole secret. Keep it in mind for step 5.

---

## 4. Sending a Message (Encoding)

**Example message: `1011`** (ON, OFF, ON, ON)

**Step 1.** Put your bulbs in seats 3, 5, 6, 7:

| Seat | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|:----:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| Bulb | ? | ? | **1** | ? | **0** | **1** | **1** |

**Step 2.** Each buddy counts the lit bulbs **of your message** that they watch. If the count is odd, the buddy lights their own bulb to make a pair.

| Buddy | Your bulbs they watch | Lit count | Even or odd? | Their own bulb |
|-------|----------------------|:---------:|:------------:|:--------------:|
| 🔴 Ruby | seats 3, 5, 7 → 1, 0, 1 | 2 | Even | **0** (OFF) |
| 🔵 Bluey | seats 3, 6, 7 → 1, 1, 1 | 3 | Odd | **1** (ON) |
| 🟢 Leafy | seats 5, 6, 7 → 0, 1, 1 | 2 | Even | **0** (OFF) |

**Step 3.** Fill in the helpers and send:

| Seat | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|:----:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| Bulb | **0** | **1** | 1 | **0** | 0 | 1 | 1 |

📦 **Sent message: `0110011`**

---

## 5. Fixing a Mistake (Correcting)

The gremlin flips **seat 6** in the tunnel. Your friend receives `0110001`.

| Seat | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|:----:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| Received | 0 | 1 | 1 | 0 | 0 | **0** 😈 | 1 |

Your friend has **no idea** a flip happened. So they ask the buddies to check. This time each buddy counts **all** lit bulbs they watch, including their own.

| Buddy | Seats watched | Lit count | Verdict | Answer |
|-------|---------------|:---------:|---------|:------:|
| 🔴 Ruby | 1, 3, 5, 7 → 0, 1, 0, 1 | 2 | Even ✅ ok | **0** |
| 🔵 Bluey | 2, 3, 6, 7 → 1, 1, 0, 1 | 3 | Odd ❌ problem | **1** |
| 🟢 Leafy | 4, 5, 6, 7 → 0, 0, 0, 1 | 1 | Odd ❌ problem | **1** |

**Read the answers as a number** in this order: **Leafy, Bluey, Ruby**

```
Leafy  Bluey  Ruby
  1      1      0     →  binary 110  →  seat 6
```

🎯 **The gremlin hit seat 6.** Your friend flips it back and gets `0110011` again. The message in seats 3, 5, 6, 7 is **`1011`** ✅

### Why does this point to the right seat?

Look back at the table in section 3. Seat 6 is watched by **Bluey and Leafy only**. When seat 6 flips, *exactly those two* buddies see an odd count, and Ruby stays happy. No other seat has that same pattern, so the pattern is a **fingerprint** of the broken seat.

| Who shouts "problem!" | Broken seat |
|-----------------------|:-----------:|
| Nobody | None 🎉 |
| Ruby | 1 |
| Bluey | 2 |
| Ruby + Bluey | 3 |
| Leafy | 4 |
| Ruby + Leafy | 5 |
| Bluey + Leafy | 6 |
| All three | 7 |

(Computer people call the pattern the **syndrome**. It's just the buddies' answers read as a number.)

---

## 6. The Limit ⚠️

Hamming code fixes **one** flipped bulb. If the gremlin flips **two**, the buddies get confused and may "fix" the wrong seat.

Real systems add **one extra bulb** that watches *all* seats. That's called **SECDED** (single-error correct, double-error detect). With it, the computer can at least say "two bulbs broke, I can't fix this, please resend."

---

## 7. Try It Yourself

### 🌐 In the browser
Open the interactive **Hamming Helpers** app. Tap bulbs, release the gremlin and watch the buddies catch him.

### ⌨️ In the terminal

```bash
gcc -std=c99 -Wall -o hamming hamming.c

./hamming                   # friendly menu (start here!)
./hamming encode 1011       # → sends 0110011
./hamming correct 0110001   # → finds the error at seat 6
./hamming demo              # play with the gremlin
./hamming story             # the story in plain words
```

If you see strange symbols, run it with `NO_COLOR=1 ./hamming`.

---

## 8. Cheat Sheet

| Idea | In plain words |
|------|----------------|
| Parity bit | A helper bulb that makes a group's lit bulbs come in pairs |
| Even parity | "Pairs": an even number of lit bulbs |
| Codeword | Your message **plus** the helper bulbs (7 bulbs total) |
| Syndrome | The buddies' ❌/✅ answers read as a number |
| Hamming(7,4) | 7 bulbs total, 4 of them are your message, 3 are helpers |
| What it can do | Fix **1** flipped bulb |
| What it can't do | Fix 2 or more flipped bulbs |

### The 3-line summary
1. **Add** 3 helper bulbs so that each buddy's group has an even number of lit bulbs.
2. **Check** the groups after the message arrives. The groups that fail point to the broken seat.
3. **Flip** that seat back. Done! ✨
