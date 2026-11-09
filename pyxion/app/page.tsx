import Image from "next/image";

import DirectMessages from "./sidebars/DM/DirectMessages";

export default function Home() {
  return (
    <div className="flex flex-row flex-1 min-h-screen w-screen bg-black">
      <DirectMessages />
    </div>
  );
}
