import type { Metadata } from "next";
import { Geist, Geist_Mono, Bangers, Fredoka, Lilita_One, Luckiest_Guy } from "next/font/google";
import "./globals.css";

const geistSans = Geist({ variable: "--font-geist-sans", subsets: ["latin"] });
const geistMono = Geist_Mono({ variable: "--font-geist-mono", subsets: ["latin"] });

const bangers = Bangers({ weight: "400", variable: "--font-bangers", subsets: ["latin"] });
const fredoka = Fredoka({ weight: ["700"], variable: "--font-fredoka", subsets: ["latin"] });
const lilita = Lilita_One({ weight: "400", variable: "--font-lilita", subsets: ["latin"] });
const luckiest = Luckiest_Guy({ weight: "400", variable: "--font-luckiest", subsets: ["latin"] });

export const metadata: Metadata = {
  title: "Pyxion",
  description: "Pyxion",
};

export default function RootLayout({ children }: LayoutProps<"/">) {
  return (
    <html
      lang="en"
      className={`${geistSans.variable} ${geistMono.variable} ${bangers.variable} ${fredoka.variable} ${lilita.variable} ${luckiest.variable} h-full antialiased`}
    >
      <body className="min-h-full flex flex-col">{children}</body>
    </html>
  );
}